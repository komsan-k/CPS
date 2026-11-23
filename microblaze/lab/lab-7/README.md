# LAB 07 — ไล่ความสว่าง LED ด้วย Timer Interrupt และ PWM

ต่อจาก [LAB 06 — PWM LED บน Nexys A7](https://chatgpt.com/library?file_id=libfile_32f51d3f76d88191ac5baf09fb134a2f&account_id=personal) โดยเพิ่มโหมดไล่ความสว่างอัตโนมัติให้ LED0 ใช้ **AXI Timer** เป็นฐานเวลา 5 ms และให้ **Verilog** สร้าง PWM 1 kHz เหมือนเดิม โปรแกรมหลักเปลี่ยนค่า duty cycle ทุก 50 ticks หรือประมาณ 250 ms พร้อมรับคำสั่งจาก Serial Terminal

**อุปกรณ์** Nexys A7-100T, USB-JTAG, โปรเจกต์ LAB 06 และ Vivado กับ Vitis Unified IDE รุ่นเดียวกัน ใช้ Classic MicroBlaze และโดเมน Standalone

| คำสั่ง | ผลที่คาดหวัง |
|---|---|
| `f` หรือ `F` | เริ่มไล่ duty cycle จาก 0% → 25% → 50% → 75% → 100% → 75% → 50% → 25% แล้ววนกลับ |
| `0`–`4` | ใช้ duty cycle คงที่ 0%, 25%, 50%, 75%, 100% ตามลำดับ |
| `s` หรือ `S` | ใช้ SW0 โดย 0 = 25% และ 1 = 75% |
| `?` | แสดงคำสั่งที่ใช้ได้ |

เริ่มระบบในโหมด SW0 เมื่อกด `f` ซ้ำ โหมดไล่ความสว่างจะเริ่มจาก 0% ใหม่ ลำดับหนึ่งรอบมี 8 ขั้น จึงใช้เวลาประมาณ 2 s

## 1. ตรวจ Block Design ใน Vivado

โปรเจกต์ LAB 06 ที่สร้างตามลำดับ LAB 03–06 สามารถเก็บ AXI Timer กับ AXI Interrupt Controller ไว้ได้ เปิดสำเนาของโปรเจกต์แล้วตรวจเส้นทางด้านล่าง หากถอดสอง IP นี้ออกไปแล้ว ให้เพิ่มและเชื่อมตาม [LAB 03 — AXI Timer Interrupt](https://chatgpt.com/library?file_id=libfile_528bf865e7a881918bfb6d7521d31755&account_id=personal)

| สัญญาณ | จุดเชื่อมต่อ |
|---|---|
| AXI Timer `interrupt` | AXI INTC `intr[0]` แบบ active-high level |
| AXI INTC `interrupt` | Classic MicroBlaze `INTERRUPT` ใน normal interrupt mode |
| AXI GPIO ช่อง 2 กว้าง 3 บิต | `led_pwm/duty_level[2:0]` |
| UART และ SW0 | พอร์ตเดิมจาก LAB 06 |

ตรวจ clock 100 MHz และ active-low reset ของ Timer และ INTC ใช้โมดูล `led_pwm.v` และ XDC จาก LAB 06 หากการเชื่อมต่อและ address ตรงกับตาราง ใช้ XSA ของ LAB 06 ได้เลย เมื่อมีการแก้ Block Design ให้ Validate Design สร้าง bitstream และ Export Hardware พร้อม Include bitstream อีกครั้ง

| IP | Base address ตัวอย่าง |
|---|---:|
| AXI GPIO | `0x40000000` |
| AXI UART Lite | `0x40600000` |
| AXI Timer | `0x41C00000` |
| AXI INTC | `0x41200000` |

## 2. เขียนโปรแกรมใน Vitis

สร้าง Empty Application สำหรับ LAB 07 บน platform ที่ตรงกับ XSA ใส่โค้ดด้านล่างเป็น `src/main.c` แล้วตรวจว่า base address ทั้งสี่ค่าตรงกับ Address Editor

```c
#include "mb_interface.h"
#include "xil_io.h"

#define GPIO_BASE         0x40000000U
#define UART_BASE         0x40600000U
#define TIMER_BASE        0x41C00000U
#define INTC_BASE         0x41200000U

#define SW_DATA           (GPIO_BASE + 0x00U)
#define PWM_SELECT        (GPIO_BASE + 0x08U)
#define SW0_MASK          0x01U

#define UART_RX           (UART_BASE + 0x00U)
#define UART_TX           (UART_BASE + 0x04U)
#define UART_STATUS       (UART_BASE + 0x08U)
#define UART_CONTROL      (UART_BASE + 0x0CU)
#define UART_RX_VALID     (1U << 0)
#define UART_TX_FULL      (1U << 3)
#define UART_RESET_FIFOS  0x03U

#define TIMER_TCSR0        (TIMER_BASE + 0x00U)
#define TIMER_TLR0         (TIMER_BASE + 0x04U)
#define TIMER_EXPIRED      (1U << 8)
#define TIMER_ENABLE       (1U << 7)
#define TIMER_IRQ_ENABLE   (1U << 6)
#define TIMER_LOAD         (1U << 5)
#define TIMER_AUTO_RELOAD  (1U << 4)
#define TIMER_DOWN_COUNT   (1U << 1)
#define TIMER_RUNNING      (TIMER_ENABLE | TIMER_IRQ_ENABLE | \
                            TIMER_AUTO_RELOAD | TIMER_DOWN_COUNT)

#define INTC_IER           (INTC_BASE + 0x08U)
#define INTC_IAR           (INTC_BASE + 0x0CU)
#define INTC_MER           (INTC_BASE + 0x1CU)
#define INTC_TIMER_IRQ     (1U << 0)
#define INTC_MASTER_ENABLE (1U << 0)
#define INTC_HW_ENABLE     (1U << 1)

#define TIMER_LOAD_5_MS    500000U
#define FADE_STEP_TICKS     50U

enum control_mode { MODE_SWITCH, MODE_MANUAL, MODE_FADE };

static const u32 fade_levels[] = {0U, 1U, 2U, 3U, 4U, 3U, 2U, 1U};
#define FADE_LEVEL_COUNT (sizeof(fade_levels) / sizeof(fade_levels[0]))

static volatile u32 timer_ticks;
static enum control_mode mode = MODE_SWITCH;
static u32 manual_level;
static u32 fade_tick;
static u32 fade_index;

static void timer_isr(void *callback_ref)
{
    (void)callback_ref;
    if ((Xil_In32(TIMER_TCSR0) & TIMER_EXPIRED) != 0U) {
        Xil_Out32(TIMER_TCSR0, TIMER_RUNNING | TIMER_EXPIRED);
        ++timer_ticks;
    }
    Xil_Out32(INTC_IAR, INTC_TIMER_IRQ);
}

static void init_timer(void)
{
    microblaze_disable_interrupts();
    Xil_Out32(INTC_MER, 0U);
    Xil_Out32(INTC_IER, 0U);
    Xil_Out32(TIMER_TCSR0, TIMER_EXPIRED);
    Xil_Out32(TIMER_TLR0, TIMER_LOAD_5_MS);
    Xil_Out32(TIMER_TCSR0, TIMER_DOWN_COUNT | TIMER_LOAD);

    microblaze_register_handler(timer_isr, 0);
    Xil_Out32(INTC_IAR, INTC_TIMER_IRQ);
    Xil_Out32(INTC_IER, INTC_TIMER_IRQ);
    Xil_Out32(INTC_MER, INTC_MASTER_ENABLE | INTC_HW_ENABLE);

    Xil_Out32(TIMER_TCSR0, TIMER_RUNNING);
    microblaze_enable_interrupts();
}

static u32 selected_level(void)
{
    if (mode == MODE_SWITCH) {
        return (Xil_In32(SW_DATA) & SW0_MASK) == 0U ? 1U : 3U;
    }
    if (mode == MODE_MANUAL) {
        return manual_level;
    }

    u32 elapsed = timer_ticks - fade_tick;
    if (elapsed >= FADE_STEP_TICKS) {
        u32 steps = elapsed / FADE_STEP_TICKS;
        fade_tick += steps * FADE_STEP_TICKS;
        fade_index = (fade_index + steps) % FADE_LEVEL_COUNT;
    }
    return fade_levels[fade_index];
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
        fade_index = 0U;
        fade_tick = timer_ticks;
        mode = MODE_FADE;
        uart_print("OK fade\r\n");
    } else if (command == '?') {
        uart_print("0: 0% | 1: 25% | 2: 50% | 3: 75% | 4: 100% | s: SW0 | f: fade\r\n");
    } else if (command != '\r' && command != '\n') {
        uart_print("Unknown command. Send ?\r\n");
    }
}

int main(void)
{
    Xil_Out32(UART_CONTROL, UART_RESET_FIFOS);

    init_timer();

    u32 applied_level = selected_level();
    Xil_Out32(PWM_SELECT, applied_level);
    uart_print("LAB 07 ready. Send ? for help.\r\n");

    for (;;) {
        if ((Xil_In32(UART_STATUS) & UART_RX_VALID) != 0U) {
            handle_command((unsigned char)Xil_In32(UART_RX));
        }

        u32 level = selected_level();
        if (level != applied_level) {
            Xil_Out32(PWM_SELECT, level);
            applied_level = level;
        }
    }
}
```

ISR ล้างสถานะของ Timer เพิ่ม `timer_ticks` แล้ว acknowledge ที่ AXI INTC โปรแกรมหลักอ่านตัวนับเพื่อตัดสินใจว่าจะเปลี่ยนขั้นเมื่อไร การลบค่า tick แบบ unsigned ทำให้การคำนวณช่วงเวลาข้ามจุดที่ตัวนับวนกลับเป็นศูนย์ได้ หากโปรแกรมหลักใช้เวลานานกว่าหนึ่งขั้นและ ISR ยังนับ tick ต่อเนื่อง โค้ดคำนวณจำนวนขั้นที่ผ่านไปจากเวลาเดิมเพื่อกลับเข้าจังหวะของลำดับ PWM รับระดับใหม่ที่ขอบคาบ 1 ms ของตัวเอง

## 3. รันและทดสอบ

ตั้ง Terminal เป็น **115200 baud, 8-N-1, flow control = None** โหลด bitstream ที่ตรงกับ XSA แล้วรันผ่าน JTAG

| การทดสอบ | ผลที่คาดหวัง |
|---|---|
| ส่ง `f` และสังเกต LED0 หลายรอบ | สว่างขึ้นและลดลงทีละขั้น วนประมาณทุก 2 s |
| ส่ง `2` ระหว่างไล่ความสว่าง | ค้างที่ duty cycle 50% |
| ส่ง `s` แล้วสลับ SW0 | LED0 สลับระดับ 25% และ 75% |
| ส่ง `f` ซ้ำในช่วงที่ LED0 สว่าง | เริ่มลำดับใหม่จาก 0% |
| ส่ง `?` ระหว่างไล่ความสว่าง | ได้ข้อความช่วยเหลือ และลำดับความสว่างเดินต่อ |

ใช้วิดีโอถ่าย LED0 และจับเวลาจากจังหวะดับครั้งแรกถึงครั้งที่หกเพื่อวัดห้ารอบ ควรได้ประมาณ 10 s หากคำสั่ง `0`–`4` ทำงานแต่โหมด `f` ค้างที่ 0% ให้ตั้ง breakpoint ที่ `timer_isr()` เพื่อตรวจการรับ interrupt จากนั้นเอา breakpoint ออกก่อนวัดเวลา

**แบบฝึกหัด** ทำให้รอบไล่ความสว่างใช้เวลาประมาณ 4 s โดยคง PWM ที่ 1 kHz คำนวณและแก้ `FADE_STEP_TICKS` เป็น 100 เพราะแต่ละขั้นต้องใช้เวลาประมาณ 500 ms

## เอกสารอ้างอิง

- [AMD AXI Timer PG079 — Generate Mode](https://docs.amd.com/r/en-US/pg079-axi-timer/Generate-Mode)
- [AMD AXI Timer PG079 — Interrupts](https://docs.amd.com/r/en-US/pg079-axi-timer/Interrupts)
- [AMD Standalone BSP UG643 — microblaze_register_handler](https://docs.amd.com/r/en-US/oslib_rm/microblaze_register_handler)
