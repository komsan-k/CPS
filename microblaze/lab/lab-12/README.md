# LAB 12 — วัดช่วงเวลาของเหตุการณ์ PWM ด้วย AXI Timer

ต่อจาก [LAB 11 — คิวเหตุการณ์ PWM](https://chatgpt.com/library?file_id=libfile_f693964557b481918926f8418831bb59&account_id=personal) ให้ ISR เก็บ **ค่าตัวนับ AXI Timer และสถานะ PWM** ลงใน FIFO เดียวกัน เมื่อนำคิวมาแสดงผ่าน UART นักศึกษาจะวัดช่วงเวลาระหว่างเหตุการณ์ได้จากเวลาที่ ISR อ่านค่า Timer แม้ข้อความจะถูกพิมพ์ในภายหลัง

**อุปกรณ์** Nexys A7-100T, USB-JTAG, โปรเจกต์ LAB 11 และ Vivado/Vitis Unified IDE รุ่นเดียวกัน ใช้ Classic MicroBlaze แบบ Standalone

```text
TICKS=10000000 DT_US=NA EVENT REQ=25% APPLIED=25%
TICKS=35000000 DT_US=250000 EVENT REQ=50% APPLIED=50%
```

`TICKS` คือค่าตัวนับดิบ 32 บิต `DT_US` คือช่วงเวลาจากรายการก่อนหน้าที่นำออกจากคิว หน่วยเป็นไมโครวินาที รายการแรกแสดง `NA` เพราะยังไม่มีค่าก่อนหน้า

## 1. ตรวจ AXI Timer ใน Vivado

1. ใช้ Block Design ของ LAB 11 ตรวจว่ามี **AXI Timer** ต่อกับ AXI interconnect เดียวกับ MicroBlaze และใช้ clock 100 MHz กับ active-low peripheral reset หากยังไม่มี IP ให้เพิ่ม AXI Timer แล้วใช้ **Run Connection Automation** ที่ S_AXI
2. ใช้ **Timer 0 ขนาด 32 บิต** ใน Generate mode แบบนับขึ้นและโหลดศูนย์ซ้ำเมื่อวนรอบ โดยตั้งค่าจากโปรแกรม C ให้ AXI GPIO ยังคงเป็นแหล่ง interrupt ที่ AXI INTC input 0 และปล่อยขา interrupt ของ Timer ไว้
3. ตรวจ base address ของ Timer ใน Address Editor ตัวอย่างใช้ `0x41C00000` หากอุปกรณ์นี้อยู่ใน XSA ของ LAB 11 แล้ว ให้ใช้ XSA และ bitstream เดิม หากเพิ่ม Timer หรือแก้ Address Editor ให้ Validate Design สร้าง bitstream และ Export Hardware พร้อม Include bitstream

## 2. สร้างเฟิร์มแวร์ใน Vitis

สร้าง Empty Application สำหรับ LAB 12 บน platform ที่ตรงกับ XSA เพิ่มไฟล์ `src/main.c` แล้วตรวจ base address ของ Timer, GPIO คำสั่ง, GPIO สถานะ, UART และ INTC ให้ตรงกับ Address Editor

```c
#include "mb_interface.h"
#include "xil_io.h"

#define TIMER_BASE        0x41C00000U
#define TIMER_TCSR0       (TIMER_BASE + 0x00U)
#define TIMER_TLR0        (TIMER_BASE + 0x04U)
#define TIMER_TCR0        (TIMER_BASE + 0x08U)
#define TIMER_EXPIRED     (1U << 8)
#define TIMER_ENABLE      (1U << 7)
#define TIMER_LOAD        (1U << 5)
#define TIMER_AUTO_RELOAD (1U << 4)
#define CLOCKS_PER_US     100U

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

struct pwm_event {
    u32 ticks;
    u32 status;
};

static volatile struct pwm_event event_queue[EVENT_CAPACITY];
static volatile u32 write_index;
static volatile u32 read_index;
static volatile u32 dropped_events;
static u32 stream_paused;
static u32 previous_ticks;
static u32 have_previous_ticks;

static void init_timestamp_timer(void)
{
    Xil_Out32(TIMER_TCSR0, TIMER_EXPIRED);
    Xil_Out32(TIMER_TLR0, 0U);
    Xil_Out32(TIMER_TCSR0, TIMER_LOAD);
    Xil_Out32(TIMER_TCSR0, TIMER_ENABLE | TIMER_AUTO_RELOAD);
}

static void enqueue_event(u32 ticks, u32 status)
{
    u32 write = write_index;
    if (write - read_index == EVENT_CAPACITY) {
        ++dropped_events;
        return;
    }
    event_queue[write & (EVENT_CAPACITY - 1U)] = (struct pwm_event){ticks, status};
    write_index = write + 1U;
}

static u32 dequeue_event(struct pwm_event *event)
{
    u32 read = read_index;
    if (read == write_index) {
        return 0U;
    }
    *event = event_queue[read & (EVENT_CAPACITY - 1U)];
    read_index = read + 1U;
    return 1U;
}

static void gpio_isr(void *callback_ref)
{
    (void)callback_ref;
    if ((Xil_In32(STATUS_IPISR) & STATUS_CH2_IRQ) != 0U) {
        u32 ticks = Xil_In32(TIMER_TCR0);
        u32 status = Xil_In32(PWM_STATUS);
        Xil_Out32(STATUS_IPISR, STATUS_CH2_IRQ);
        enqueue_event(ticks, status);
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

static void print_event(const struct pwm_event *event)
{
    uart_print("TICKS=");
    uart_print_u32(event->ticks);
    uart_print(" DT_US=");
    if (have_previous_ticks != 0U) {
        uart_print_u32((event->ticks - previous_ticks) / CLOCKS_PER_US);
    } else {
        uart_print("NA");
    }
    previous_ticks = event->ticks;
    have_previous_ticks = 1U;
    print_pwm(" EVENT ", event->status);
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
        struct pwm_event event;
        if (dequeue_event(&event) == 0U) {
            break;
        }
        print_event(&event);
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

    init_timestamp_timer();
    init_gpio_interrupt();

    u32 applied_control = selected_control();
    Xil_Out32(PWM_SELECT, applied_control);
    uart_print("LAB 12 ready. Send ? for help.\r\n");

    for (;;) {
        if ((Xil_In32(UART_STATUS) & UART_RX_VALID) != 0U) {
            handle_command((unsigned char)Xil_In32(UART_RX));
        }

        u32 control = selected_control();
        if (control != applied_control) {
            Xil_Out32(PWM_SELECT, control);
            applied_control = control;
        }

        struct pwm_event event;
        if (stream_paused == 0U && dequeue_event(&event) != 0U) {
            print_event(&event);
        }
    }
}
```

AXI Timer รับ clock 100 MHz จึงมี 100 ticks ต่อไมโครวินาที ผลต่างแบบ unsigned `ticks_now - ticks_previous` ใช้วัดข้ามจุดที่ตัวนับวนกลับเป็นศูนย์ได้เมื่อช่วงเวลาจริงสั้นกว่าหนึ่งรอบตัวนับ หรือประมาณ 42.95 s หากเว้นระหว่างสองเหตุการณ์นานกว่านั้น จะต้องมีข้อมูลเพิ่มเพื่อระบุจำนวนรอบที่ผ่านไป ค่าที่ได้รวมความแปรผันของเวลาตอบสนอง interrupt และเวลาที่ใช้ในการอ่านรีจิสเตอร์

## 3. ทดสอบบน Nexys A7

โหลด bitstream และรันผ่าน USB-JTAG ตั้ง Serial Terminal เป็น **115200 baud, 8-N-1, flow control = None**

| การทดสอบ | ขั้นตอนและผลที่คาดหวัง |
|---|---|
| วัดรอบไล่ความสว่าง | ส่ง `f` แล้วอ่านหลายบรรทัด หลังผ่านขั้นแรก ค่า DT_US ควรอยู่ใกล้ 250,000 µs ต่อขั้น ผลรวมแปดช่วงติดกันควรอยู่ใกล้ 2,000,000 µs |
| วัดย้อนหลังจากคิว | ขณะไล่ระดับให้ส่ง `p` พักประมาณ 2 s แล้วส่ง `l` รายการที่อ่านย้อนหลังควรยังห่างกันประมาณ 250,000 µs |
| ตรวจสถานะปัจจุบัน | ส่ง `r` เพื่ออ่าน REQ/APPLIED ทันที แล้วใช้ `q` ตรวจว่า DROPPED ยังเป็นศูนย์ก่อนนำผลไปคำนวณ |
| ตรวจตัวนับวนรอบ | ปล่อยโหมด `f` ทำงานเกิน 43 s โดยเปิดแสดงผลสด เมื่อ TICKS กลับไปใกล้ศูนย์ ค่า DT_US ของสองเหตุการณ์ติดกันควรยังอยู่ใกล้ 250,000 µs |

หาก `DROPPED` เพิ่มขึ้น ช่วงเวลาระหว่างสองรายการที่เก็บไว้ได้อาจรวมหลายขั้นของการไล่ความสว่าง ให้ใช้ช่วงที่ไม่มีการทิ้งรายการเมื่อต้องการวัดเวลาต่อขั้น

**แบบฝึกหัด** เก็บข้อมูลต่อเนื่องอย่างน้อย 20 ช่วงหลังเริ่มโหมด `f` แล้วคำนวณค่าต่ำสุด สูงสุด และค่าเฉลี่ยของ DT_US เปรียบเทียบผลในโหมดแสดงสดกับการพักคิวช่วงสั้น ๆ โดยดู DROPPED ประกอบ

## เอกสารอ้างอิง

- [AMD AXI Timer PG079 — Generate Mode](https://docs.amd.com/r/en-US/pg079-axi-timer/Generate-Mode)
- [AMD AXI Timer PG079 — Register Space](https://docs.amd.com/r/en-US/pg079-axi-timer/Register-Space)
