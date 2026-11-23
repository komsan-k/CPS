# LAB 04 — ควบคุม LED ผ่าน AXI UART Lite บน Nexys A7

ต่อจาก [LAB 03 — AXI Timer Interrupt](https://chatgpt.com/library?file_id=libfile_528bf865e7a881918bfb6d7521d31755&account_id=personal) โดยเพิ่ม AXI UART Lite ให้ MicroBlaze รับคำสั่งจาก Serial Terminal บนคอมพิวเตอร์ โปรแกรมหลักรับอักขระและส่งข้อความตอบกลับ ขณะที่ Timer ISR ยังคงกำหนดจังหวะกะพริบของ LED0 ทุกครึ่งคาบ

**อุปกรณ์** Nexys A7-100T, สาย USB แบบส่งข้อมูลสำหรับพอร์ต J6, โปรเจกต์ LAB 03, Vivado และ Vitis Unified IDE รุ่นเดียวกัน และโปรแกรม Serial Terminal เช่น Tera Term หรือ PuTTY

| อักขระ | การทำงาน |
|---|---|
| 1 | บังคับ LED0 ให้กะพริบประมาณ 1 Hz |
| 4 | บังคับ LED0 ให้กะพริบประมาณ 4 Hz |
| s หรือ S | กลับไปใช้ SW0 เลือก 1 Hz หรือ 4 Hz เหมือน LAB 03 |
| ? | แสดงรายการคำสั่ง |

เริ่มต้นในโหมด SW0 คำสั่งมีผลกับช่วงติดหรือดับถัดไป และตอบกลับทันทีที่โปรแกรมรับคำสั่ง ส่งทีละอักขระได้โดยไม่ต้องกด Enter

## 1. เพิ่ม AXI UART Lite ใน Vivado

1. เปิดสำเนาโปรเจกต์ LAB 03 และเปิด Block Design เพิ่ม **AXI UART Lite** แล้วใช้ **Run Connection Automation** เชื่อม S_AXI เข้ากับ AXI interconnect ของ MicroBlaze ใช้ clock 100 MHz และ peripheral reset แบบ active-low ชุดเดิม
2. ตั้ง AXI UART Lite เป็น **115200 baud, 8 data bits, no parity** ใช้การสื่อสารแบบ 8-N-1 ที่ฝั่ง Terminal ปล่อยขา interrupt ของ UART ไว้ เพราะโปรแกรมหลักอ่านสถานะ RX FIFO และ Timer เป็นแหล่ง interrupt เดียวของ AXI INTC
3. ใช้ **Make External** ที่อินเทอร์เฟซ UART ตั้งชื่ออินเทอร์เฟซภายนอกเป็น uart แล้วตรวจชื่อพอร์ตที่สร้างจริงใน HDL wrapper ตัวอย่างด้านล่างใช้ uart_rxd และ uart_txd หากชื่อใน wrapper ต่างออกไป ให้แก้ชื่อใน XDC ตามจริง ขา C4 เป็นอินพุต RX ของ FPGA และ D4 เป็นเอาต์พุต TX ของ FPGA
4. เพิ่มสองบรรทัดนี้ใน XDC ของ LAB 03 หาก Vivado ผูกขา USB UART ผ่าน Board Interface และมี constraints แล้ว ให้ใช้การผูกขาเพียงชุดเดียว

```tcl
set_property -dict {PACKAGE_PIN C4 IOSTANDARD LVCMOS33} [get_ports {uart_rxd}]
set_property -dict {PACKAGE_PIN D4 IOSTANDARD LVCMOS33} [get_ports {uart_txd}]
```

5. ตรวจ **Address Editor** ให้ AXI UART Lite อยู่ใน data address space ของ MicroBlaze พร้อมกับอุปกรณ์เดิม ค่าตัวอย่างอยู่ในโค้ดด้านล่าง หาก Vivado กำหนดค่าอื่น ให้ใช้ค่าจริงในโค้ด
6. เลือก **Validate Design → Generate Bitstream → Export Hardware** และเลือก **Include bitstream** เพื่อสร้าง XSA สำหรับ LAB 04

## 2. เขียนโปรแกรมใน Vitis

สร้าง **Platform Component** จาก XSA ของ LAB 04 เลือก Classic MicroBlaze และ Standalone แล้ว build platform สร้าง **Empty Application** บน platform นี้ แล้วเพิ่มโค้ดต่อไปนี้เป็น src/main.c

```c
#include "mb_interface.h"
#include "xil_io.h"

#define GPIO_BASE          0x40000000U
#define TIMER_BASE         0x41C00000U
#define INTC_BASE          0x41200000U
#define UART_BASE          0x40600000U

#define SW_DATA            (GPIO_BASE + 0x00U)
#define LED_DATA           (GPIO_BASE + 0x08U)
#define GPIO_BIT           0x01U

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
#define INTC_ENABLE        0x03U  /* Master enable and hardware enable. */

#define UART_RX            (UART_BASE + 0x00U)
#define UART_TX            (UART_BASE + 0x04U)
#define UART_STATUS        (UART_BASE + 0x08U)
#define UART_CONTROL       (UART_BASE + 0x0CU)
#define UART_RX_VALID      (1U << 0)
#define UART_TX_FULL       (1U << 3)
#define UART_RESET_FIFOS   0x03U

#define TIMER_LOAD_5_MS    500000U
#define SLOW_IRQS          100U
#define FAST_IRQS          25U

enum { MODE_SWITCH, MODE_SLOW, MODE_FAST };

static volatile u32 requested_mode = MODE_SWITCH;
static u32 irqs_remaining;
static u32 led_state;

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
    switch (command) {
    case '1':
        requested_mode = MODE_SLOW;
        uart_print("OK 1 Hz\r\n");
        break;
    case '4':
        requested_mode = MODE_FAST;
        uart_print("OK 4 Hz\r\n");
        break;
    case 's':
    case 'S':
        requested_mode = MODE_SWITCH;
        uart_print("OK SW0 mode\r\n");
        break;
    case '?':
        uart_print("1: 1 Hz | 4: 4 Hz | s: SW0\r\n");
        break;
    case '\r':
    case '\n':
        break;
    default:
        uart_print("Unknown command. Send ?\r\n");
        break;
    }
}

static u32 read_half_period(void)
{
    u32 mode = requested_mode;
    if (mode == MODE_SWITCH) {
        return (Xil_In32(SW_DATA) & GPIO_BIT) == 0U ? SLOW_IRQS : FAST_IRQS;
    }
    return mode == MODE_SLOW ? SLOW_IRQS : FAST_IRQS;
}

static void timer_isr(void *callback_ref)
{
    (void)callback_ref;

    if ((Xil_In32(TIMER_TCSR0) & TIMER_EXPIRED) != 0U) {
        Xil_Out32(TIMER_TCSR0, TIMER_RUNNING | TIMER_EXPIRED);
        if (--irqs_remaining == 0U) {
            led_state ^= GPIO_BIT;
            Xil_Out32(LED_DATA, led_state);
            irqs_remaining = read_half_period();
        }
    }

    Xil_Out32(INTC_IAR, INTC_TIMER_IRQ);
}

int main(void)
{
    microblaze_disable_interrupts();

    Xil_Out32(UART_CONTROL, UART_RESET_FIFOS);
    Xil_Out32(INTC_MER, 0U);
    Xil_Out32(INTC_IER, 0U);
    Xil_Out32(TIMER_TCSR0, TIMER_EXPIRED);
    Xil_Out32(TIMER_TLR0, TIMER_LOAD_5_MS);
    Xil_Out32(TIMER_TCSR0, TIMER_DOWN_COUNT | TIMER_LOAD);

    Xil_Out32(LED_DATA, led_state);
    irqs_remaining = read_half_period();

    microblaze_register_handler(timer_isr, 0);
    Xil_Out32(INTC_IAR, INTC_TIMER_IRQ);
    Xil_Out32(INTC_IER, INTC_TIMER_IRQ);
    Xil_Out32(INTC_MER, INTC_ENABLE);
    Xil_Out32(TIMER_TCSR0, TIMER_RUNNING);
    microblaze_enable_interrupts();

    uart_print("LAB 04 ready. Send ? for help.\r\n");

    for (;;) {
        if ((Xil_In32(UART_STATUS) & UART_RX_VALID) != 0U) {
            unsigned char command = (unsigned char)Xil_In32(UART_RX);
            handle_command(command);
        }
    }
}
```

โปรแกรมหลักเขียน requested_mode และ ISR อ่านค่าหนึ่งครั้งที่ขอบของ LED0 ตัวแปรโหมดเป็น volatile เพื่อให้ ISR อ่านค่าจากหน่วยความจำตามที่เขียนในโปรแกรม ส่วน led_state และตัวนับอยู่ใน ISR หลังการตั้งค่าเริ่มต้น การส่งข้อความอาจรอพื้นที่ใน TX FIFO ได้ในโปรแกรมหลัก ขณะที่ Timer interrupt ยังทำงาน

## 3. รันและตรวจผล

เปิด Serial Terminal บนพอร์ตของบอร์ด ตั้ง **115200 baud, 8-N-1, flow control = None** แล้ว build และรันโปรแกรมผ่าน JTAG หากไม่เห็นข้อความเริ่มต้น ให้ส่ง ? เพื่อขอรายการคำสั่งใหม่ หากไม่เห็นตัวอักษรที่พิมพ์แต่เห็นข้อความตอบกลับ ถือว่าทำงานตามปกติเมื่อปิด local echo

| การทดสอบ | ผลที่คาดหวัง |
|---|---|
| ตั้ง SW0 = 0 แล้วส่ง 4 | ตอบ OK 4 Hz และ LED0 กะพริบประมาณ 4 Hz |
| ตั้ง SW0 = 1 แล้วส่ง 1 | ตอบ OK 1 Hz และ LED0 กะพริบประมาณ 1 Hz |
| ส่ง s แล้วเปลี่ยน SW0 จาก 1 เป็น 0 | ตอบ OK SW0 mode และความถี่เปลี่ยนจากประมาณ 4 Hz เป็น 1 Hz |
| ส่ง ? แล้วส่ง x | ได้รายการคำสั่งและข้อความแจ้งว่าไม่รู้จักคำสั่งตามลำดับ |

เมื่อวัดห้าคาบเต็ม ควรได้ประมาณ 5 s ที่ 1 Hz และ 1.25 s ที่ 4 Hz ถ้า LED0 ทำงานแต่ UART เงียบ ให้ตรวจพอร์ต Serial Terminal, การตั้งค่า 115200 8-N-1, ทิศทาง RX/TX ใน XDC และ UART_BASE ใน Address Editor

**แบบฝึกหัด** เพิ่มคำสั่ง 2 สำหรับความถี่ 2 Hz โดยเพิ่มโหมดและข้อความในเมนู ใช้ **50 Timer interrupts ต่อการสลับ LED0 หนึ่งครั้ง** และทดสอบว่าห้าคาบใช้เวลาประมาณ 2.5 s

## เอกสารอ้างอิง

- [AMD AXI UART Lite Product Guide PG142](https://docs.amd.com/api/khub/documents/dB1MAeh~uLG7FE62a5_QbA/content)
- [Digilent Nexys A7 Reference Manual](https://digilent.com/reference/_media/reference/programmable-logic/nexys-a7/nexys-a7_rm.pdf)
- [Digilent Nexys A7-100T Master XDC](https://github.com/Digilent/digilent-xdc/blob/master/Nexys-A7-100T-Master.xdc)
