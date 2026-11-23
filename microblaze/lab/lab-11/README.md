# LAB 11 — เก็บเหตุการณ์ PWM ด้วย Ring Buffer บน MicroBlaze

ต่อจาก [LAB 10 — AXI GPIO Interrupt](https://chatgpt.com/library?file_id=libfile_769b907e23bc8191a50fac613f8ac0e4&account_id=personal) เมื่อระดับ PWM ที่ใช้งานจริงเปลี่ยน ISR จะอ่าน REQ และ APPLIED หนึ่งครั้งแล้วเก็บค่าลง **คิว FIFO ขนาด 16 รายการ** โปรแกรมหลักจึงสามารถพิมพ์เหตุการณ์ตามลำดับ แม้จะพักการแสดงผล UART ไว้ชั่วคราว

**อุปกรณ์** Nexys A7-100T, USB-JTAG, XSA และ bitstream จาก LAB 10, Vitis Unified IDE รุ่นเดียวกับ Vivado ที่สร้าง XSA ใช้ Classic MicroBlaze แบบ Standalone

| คำสั่ง | การทำงาน |
|---|---|
| `p` | สลับระหว่างแสดงผลสดกับพักการแสดงผล โดย ISR ยังเก็บเหตุการณ์ |
| `l` | พิมพ์และนำรายการที่อยู่ในคิวตอนรับคำสั่งออกตามลำดับ |
| `q` | ดูจำนวนรายการในคิว (QUEUED) และจำนวนที่ทิ้งสะสม (DROPPED) |
| `r` | อ่านสถานะ PWM ปัจจุบัน |
| `0`–`4`, `s`, `f`, `?` | ตั้งค่า PWM และเปิดเมนูตาม LAB 10 |

## 1. ใช้ฮาร์ดแวร์ LAB 10

ใช้ XSA และ bitstream ของ LAB 10 โดยตรง ตรวจว่า AXI GPIO สถานะช่อง 1 ยังส่ง REQ และ APPLIED รวมกัน และ interrupt มาจากการเปลี่ยน APPLIED ที่ช่อง 2 ผ่าน AXI INTC input 0 สร้าง Empty Application แยกสำหรับ LAB 11 บน platform แบบ Standalone

## 2. รันเฟิร์มแวร์ LAB 11

บันทึกโค้ดเป็น `src/main.c` แล้วตรวจ base address ของ GPIO, GPIO สถานะ, UART และ INTC ให้ตรงกับ Address Editor ของ LAB 10

```c
#include "mb_interface.h"
#include "xil_io.h"

#define GPIO_BASE         0x40000000U
#define STATUS_GPIO_BASE  0x40010000U
#define UART_BASE         0x40600000U

#define SW_DATA           (GPIO_BASE + 0x00U)
#define PWM_SELECT        (GPIO_BASE + 0x08U)
#define PWM_STATUS        (STATUS_GPIO_BASE + 0x00U)
#define LEVEL_MASK        0x07U
#define REQUESTED_SHIFT   3U
#define SW0_MASK          0x01U

#define UART_RX           (UART_BASE + 0x00U)
#define UART_TX           (UART_BASE + 0x04U)
#define UART_STATUS       (UART_BASE + 0x08U)
#define UART_CONTROL      (UART_BASE + 0x0CU)
#define UART_RX_VALID     (1U << 0)
#define UART_TX_FULL      (1U << 3)
#define UART_RESET_FIFOS  0x03U

#define STATUS_GIER        (STATUS_GPIO_BASE + 0x11CU)
#define STATUS_IPISR       (STATUS_GPIO_BASE + 0x120U)
#define STATUS_IPIER       (STATUS_GPIO_BASE + 0x128U)
#define STATUS_CH2_IRQ     (1U << 1)
#define STATUS_GLOBAL_IRQ  (1U << 31)

#define INTC_BASE          0x41200000U
#define INTC_IER           (INTC_BASE + 0x08U)
#define INTC_IAR           (INTC_BASE + 0x0CU)
#define INTC_MER           (INTC_BASE + 0x1CU)
#define INTC_GPIO_IRQ      (1U << 0)
#define INTC_ENABLE        0x03U

#define FADE_ENABLE  (1U << 3)
#define FADE_RESTART (1U << 4)

enum control_mode { MODE_SWITCH, MODE_MANUAL, MODE_FADE };

static enum control_mode mode = MODE_SWITCH;
static u32 manual_level;
static u32 restart_token;
#define EVENT_CAPACITY 16U

static volatile u32 event_queue[EVENT_CAPACITY];
static volatile u32 write_index;
static volatile u32 read_index;
static volatile u32 dropped_events;
static u32 stream_paused;

static void enqueue_event(u32 status)
{
    u32 write = write_index;
    if (write - read_index == EVENT_CAPACITY) {
        ++dropped_events;
        return;
    }
    event_queue[write & (EVENT_CAPACITY - 1U)] = status;
    write_index = write + 1U;
}

static u32 dequeue_event(u32 *status)
{
    u32 read = read_index;
    if (read == write_index) {
        return 0U;
    }
    *status = event_queue[read & (EVENT_CAPACITY - 1U)];
    read_index = read + 1U;
    return 1U;
}

static void gpio_isr(void *callback_ref)
{
    (void)callback_ref;
    if ((Xil_In32(STATUS_IPISR) & STATUS_CH2_IRQ) != 0U) {
        Xil_Out32(STATUS_IPISR, STATUS_CH2_IRQ);
        enqueue_event(Xil_In32(PWM_STATUS));
    }
    Xil_Out32(INTC_IAR, INTC_GPIO_IRQ);
}

static void init_gpio_interrupt(void)
{
    microblaze_disable_interrupts();
    Xil_Out32(INTC_MER, 0U);
    Xil_Out32(INTC_IER, 0U);
    Xil_Out32(STATUS_GIER, 0U);
    Xil_Out32(STATUS_IPIER, 0U);

    u32 pending = Xil_In32(STATUS_IPISR) & STATUS_CH2_IRQ;
    Xil_Out32(STATUS_IPISR, pending);
    Xil_Out32(INTC_IAR, INTC_GPIO_IRQ);
    microblaze_register_handler(gpio_isr, 0);

    Xil_Out32(STATUS_IPIER, STATUS_CH2_IRQ);
    Xil_Out32(STATUS_GIER, STATUS_GLOBAL_IRQ);
    Xil_Out32(INTC_IER, INTC_GPIO_IRQ);
    Xil_Out32(INTC_MER, INTC_ENABLE);
    microblaze_enable_interrupts();
}

static u32 selected_control(void)
{
    if (mode == MODE_FADE) {
        return FADE_ENABLE | restart_token;
    }

    u32 level = manual_level;
    if (mode == MODE_SWITCH) {
        level = (Xil_In32(SW_DATA) & SW0_MASK) == 0U ? 1U : 3U;
    }
    return restart_token | level;
}

static void uart_print(const char *message)
{
    while (*message != '\0') {
        while ((Xil_In32(UART_STATUS) & UART_TX_FULL) != 0U) {
        }
        Xil_Out32(UART_TX, (unsigned char)*message);
        ++message;
    }
}

static void print_pwm(const char *prefix, u32 status)
{
    static const char *const values[] = {"0%", "25%", "50%", "75%", "100%"};

    u32 applied = status & LEVEL_MASK;
    u32 requested = (status >> REQUESTED_SHIFT) & LEVEL_MASK;

    uart_print(prefix);
    uart_print("REQ=");
    uart_print(requested <= 4U ? values[requested] : "invalid");
    uart_print(" APPLIED=");
    uart_print(applied <= 4U ? values[applied] : "invalid");
    uart_print("\r\n");
}

static void uart_print_u32(u32 value)
{
    char digits[11];
    unsigned int pos = sizeof(digits) - 1U;
    digits[pos] = '\0';
    do {
        digits[--pos] = (char)('0' + value % 10U);
        value /= 10U;
    } while (value != 0U);
    uart_print(&digits[pos]);
}

static void print_queue_status(void)
{
    u32 read = read_index;
    u32 queued = write_index - read;
    u32 dropped = dropped_events;
    uart_print("QUEUED=");
    uart_print_u32(queued);
    uart_print(" DROPPED=");
    uart_print_u32(dropped);
    uart_print("\r\n");
}

static void dump_events(void)
{
    u32 read = read_index;
    u32 count = write_index - read;
    for (u32 i = 0U; i < count; ++i) {
        u32 status;
        if (dequeue_event(&status) == 0U) {
            break;
        }
        print_pwm("EVENT ", status);
    }
    print_queue_status();
}

static void handle_command(unsigned char command)
{
    static const char *const replies[] = {
        "OK 0%\r\n", "OK 25%\r\n", "OK 50%\r\n",
        "OK 75%\r\n", "OK 100%\r\n"
    };

    if (command >= '0' && command <= '4') {
        manual_level = (u32)(command - '0');
        mode = MODE_MANUAL;
        uart_print(replies[manual_level]);
    } else if (command == 's' || command == 'S') {
        mode = MODE_SWITCH;
        uart_print("OK SW0 mode\r\n");
    } else if (command == 'f' || command == 'F') {
        restart_token ^= FADE_RESTART;
        mode = MODE_FADE;
        uart_print("OK fade\r\n");
    } else if (command == 'r' || command == 'R') {
        print_pwm("", Xil_In32(PWM_STATUS));
    } else if (command == 'p' || command == 'P') {
        stream_paused ^= 1U;
        uart_print(stream_paused != 0U ? "STREAM PAUSED\r\n" : "STREAM LIVE\r\n");
    } else if (command == 'l' || command == 'L') {
        dump_events();
    } else if (command == 'q' || command == 'Q') {
        print_queue_status();
    } else if (command == '?') {
        uart_print("0: 0% | 1: 25% | 2: 50% | 3: 75% | 4: 100% | s: SW0 | f: fade | r: read PWM\r\n");
        uart_print("p: pause/resume | l: list and drain | q: queue status\r\n");
    } else if (command != '\r' && command != '\n') {
        uart_print("Unknown command. Send ?\r\n");
    }
}

int main(void)
{
    Xil_Out32(UART_CONTROL, UART_RESET_FIFOS);

    init_gpio_interrupt();

    u32 applied_control = selected_control();
    Xil_Out32(PWM_SELECT, applied_control);
    uart_print("LAB 11 ready. Send ? for help.\r\n");

    for (;;) {
        if ((Xil_In32(UART_STATUS) & UART_RX_VALID) != 0U) {
            handle_command((unsigned char)Xil_In32(UART_RX));
        }

        u32 control = selected_control();
        if (control != applied_control) {
            Xil_Out32(PWM_SELECT, control);
            applied_control = control;
        }

        u32 status;
        if (stream_paused == 0U && dequeue_event(&status) != 0U) {
            print_pwm("EVENT ", status);
        }
    }
}
```

ISR เป็นผู้เขียนคิวเพียงรายเดียว โดยเขียนข้อมูลลงช่องก่อนเลื่อน `write_index` โปรแกรมหลักเป็นผู้อ่านเพียงรายเดียวและคัดลอกข้อมูลก่อนเลื่อน `read_index` การพิมพ์ UART เกิดหลังนำข้อมูลออกจากคิว ผลต่างของดัชนีสองตัวคือจำนวนรายการ และ unsigned arithmetic ทำให้ดัชนียังนับต่อได้เมื่อวนกลับเป็นศูนย์ การใช้ bit mask สำหรับเลือกช่องอาศัยความจุที่เป็นกำลังของ 2

เมื่อคิวเต็ม ระบบเก็บ 16 รายการเดิมและเพิ่ม `DROPPED` สำหรับแต่ละเหตุการณ์ใหม่ที่เก็บไม่ได้ เหตุการณ์ที่เกิดระหว่างพิมพ์ผลของ `l` จะอยู่ในคิวสำหรับการอ่านครั้งถัดไปถ้ายังมีพื้นที่ ค่านี้นับเฉพาะรายการที่ ISR พบแต่คิวเต็ม

## 3. ทดสอบบน Nexys A7

ใช้ USB-JTAG โหลด bitstream และรันโปรแกรม ตั้ง Serial Terminal เป็น **115200 baud, 8-N-1, flow control = None** เริ่มทดสอบจากการรันโปรแกรมใหม่เพื่อให้ DROPPED เริ่มที่ศูนย์

| กรณี | ขั้นตอน | ผลที่คาดหวัง |
|---|---|---|
| ตรวจลำดับ FIFO | ส่ง `0` แล้วรอให้คงที่ ส่ง `p` จากนั้นส่ง `1`, `3`, `2` ทีละตัว โดยเว้นแต่ละคำสั่งให้ LED เปลี่ยนแล้ว ส่ง `q` และ `l` | ก่อนอ่านได้ `QUEUED=3 DROPPED=0` แล้วเห็น APPLIED 25%, 75%, 50% ตามลำดับ และคิวกลับเป็นศูนย์ |
| แยกสถานะสดจากข้อมูลในคิว | ยังอยู่ในโหมดพัก ส่ง `1` แล้ว `4` รอให้ LED คงที่ ส่ง `r` ตามด้วย `l` | `r` แสดง 100% ส่วน `l` พิมพ์ค่าที่เก็บไว้ 25% แล้ว 100% |
| ทดสอบคิวเต็ม | ขณะพักและคิวว่าง ส่ง `f` รออย่างน้อย 6 s แล้วส่ง `2` เพื่อหยุดไล่ระดับ รอให้ LED คงที่แล้วส่ง `q` | `QUEUED=16` และ `DROPPED` มากกว่า 0 ส่ง `l` จะเห็น 16 รายการเดิมตามลำดับ |
| กลับสู่โหมดสด | ส่ง `p` แล้วส่ง `1` | ได้ `STREAM LIVE` และข้อความ EVENT เมื่อระดับ PWM เปลี่ยน |

**แบบฝึกหัด** เปลี่ยนความจุจาก 16 เป็น 32 แล้วคำนวณว่าคิวว่างจะรองรับการไล่ความสว่างได้นานประมาณกี่วินาทีเมื่อพัก UART คำตอบประมาณ 8 s เพราะมีเหตุการณ์หนึ่งครั้งต่อ 250 ms

## เอกสารอ้างอิง

- [AMD AXI GPIO PG144 — Interrupts](https://docs.amd.com/r/en-US/pg144-axi-gpio/Interrupts)
