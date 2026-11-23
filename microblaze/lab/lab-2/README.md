# LAB 02 — AXI Timer และ LED กะพริบบน Nexys A7

ต่อยอดจาก [LAB 01 — MicroBlaze และ AXI GPIO](https://chatgpt.com/library?file_id=libfile_e92c6ff5e4048191801f7c73ab481da3&account_id=personal) โดยเพิ่ม **AXI Timer** ให้ MicroBlaze กำหนดจังหวะกะพริบของ LED0 และใช้ SW0 เลือกความเร็ว

**อุปกรณ์** Nexys A7-100T, สาย USB-JTAG, โปรเจกต์ LAB 01 และ Vivado กับ Vitis Unified IDE รุ่นเดียวกัน ใช้ Classic MicroBlaze และโดเมน Standalone

| SW0 | เวลาระหว่างการสลับสถานะ LED0 | คาบเต็ม ติดแล้วดับครบหนึ่งรอบ | ความถี่ |
|---|---:|---:|---:|
| 0 | ประมาณ 500 ms | ประมาณ 1 s | 1 Hz |
| 1 | ประมาณ 125 ms | ประมาณ 250 ms | 4 Hz |

## 1. เพิ่ม AXI Timer ใน Vivado

1. เปิดสำเนาโปรเจกต์ LAB 01 แล้วเปิด Block Design เดิม ตรวจว่า AXI GPIO ช่อง 1 รับ SW0 และช่อง 2 ส่งออก LED0
2. เพิ่ม IP **AXI Timer** แล้วใช้ **Run Connection Automation** ที่ `S_AXI` เชื่อม Timer เข้ากับ AXI interconnect เดียวกับ AXI GPIO ใช้ Counter 0 ความกว้าง 32 บิต
3. ตรวจว่า `s_axi_aclk` ของ Timer รับ clock 100 MHz และ `s_axi_aresetn` รับ reset แบบ active-low จาก Processor System Reset เช่นเดียวกับ AXI GPIO ใน LAB นี้ใช้ polling จึงปล่อยขา `interrupt` ไว้ได้
4. เปิด **Address Editor** และตรวจ base address ใน data address space ของ MicroBlaze

| IP | Base address ที่ใช้ในโค้ด |
|---|---:|
| AXI GPIO | `0x40000000` |
| AXI Timer | `0x41C00000` |

ถ้า Vivado กำหนด address อื่น ให้ใช้ค่าจริงจาก Address Editor ใน `GPIO_BASE` และ `TIMER_BASE`

5. ใช้ XDC ของ LAB 01 เพราะ Timer ต่ออยู่ภายใน FPGA จากนั้น **Validate Design**, สร้าง bitstream ใหม่ และ **Export Hardware** พร้อม **Include bitstream** เป็น XSA สำหรับ LAB 02

## 2. เขียนโปรแกรมใน Vitis

สร้าง Platform Component จาก **XSA ของ LAB 02** เลือก MicroBlaze และ Standalone แล้ว build platform สร้าง **Empty Application** บน platform นี้ ใส่โค้ดต่อไปนี้เป็น `src/main.c`

```c
#include "xil_io.h"

#define GPIO_BASE         0x40000000U
#define TIMER_BASE        0x41C00000U

#define SW_DATA_OFFSET    0x00U
#define LED_DATA_OFFSET   0x08U
#define GPIO_BIT_MASK     0x01U

#define TIMER_TCSR0       0x00U
#define TIMER_TLR0        0x04U
#define TIMER_EXPIRED     (1U << 8)
#define TIMER_ENABLE      (1U << 7)
#define TIMER_LOAD        (1U << 5)
#define TIMER_DOWN_COUNT  (1U << 1)

#define SLOW_TICKS        50000000U
#define FAST_TICKS        12500000U

static void wait_for_timer(u32 ticks)
{
    /* Stop Timer 0 and clear its previous expiration flag. */
    Xil_Out32(TIMER_BASE + TIMER_TCSR0, TIMER_EXPIRED);
    Xil_Out32(TIMER_BASE + TIMER_TLR0, ticks);
    Xil_Out32(TIMER_BASE + TIMER_TCSR0, TIMER_DOWN_COUNT | TIMER_LOAD);
    Xil_Out32(TIMER_BASE + TIMER_TCSR0, TIMER_DOWN_COUNT | TIMER_ENABLE);

    /* Poll the status bit until Timer 0 rolls over. */
    while ((Xil_In32(TIMER_BASE + TIMER_TCSR0) & TIMER_EXPIRED) == 0U) {
    }
}

int main(void)
{
    u32 led_state = 0U;
    Xil_Out32(GPIO_BASE + LED_DATA_OFFSET, led_state);

    for (;;) {
        u32 sw0 = Xil_In32(GPIO_BASE + SW_DATA_OFFSET) & GPIO_BIT_MASK;
        u32 ticks = (sw0 == 0U) ? SLOW_TICKS : FAST_TICKS;

        wait_for_timer(ticks);
        led_state ^= GPIO_BIT_MASK;
        Xil_Out32(GPIO_BASE + LED_DATA_OFFSET, led_state);
    }
}
```

เมื่อ clock เป็น 100 MHz หนึ่ง tick ใช้เวลา 10 ns ดังนั้น 50,000,000 ticks ใช้เวลาประมาณ 500 ms ส่วน 12,500,000 ticks ใช้เวลาประมาณ 125 ms โปรแกรมโหลดค่าลง Timer 0 แล้วเริ่มนับถอยหลังใน Generate mode เมื่อ counter เกิด rollover บิตสถานะ `T0INT` จะเป็น 1 และโปรแกรมจะสลับ LED0 การเริ่มรอบถัดไปจะเขียน 1 ไปที่บิตนี้เพื่อล้างสถานะเดิม ค่าเวลาบนบอร์ดอาจเพิ่มขึ้นเล็กน้อยจากการโหลด Timer และการอ่านเขียนรีจิสเตอร์

## 3. รันและตรวจผล

1. ต่อ USB-JTAG เปิดบอร์ด แล้ว build และรัน application บนฮาร์ดแวร์ หากเริ่มผ่าน debugger ให้กด **Resume** เมื่อหยุดที่ `main()`
2. ตั้ง SW0 = 0 และวัดเวลารวมตั้งแต่ขอบขาขึ้นครั้งแรกถึงขอบขาขึ้นครั้งที่หก ควรได้ประมาณ 5 s สำหรับห้าคาบ
3. ตั้ง SW0 = 1 แล้ววัดแบบเดิม ควรได้ประมาณ 1.25 s สำหรับห้าคาบ โปรแกรมจะใช้ค่าของ SW0 เมื่อเริ่มช่วงจับเวลาถัดไป

หาก LED0 ค้าง ให้ตรวจว่า Vitis ใช้ XSA ของ LAB 02, base address ตรงกับ Address Editor และ Timer ได้รับ clock กับ reset หาก LED0 กะพริบแต่ความถี่คลาดเคลื่อนมาก ให้ตรวจความถี่จริงของ `s_axi_aclk` การวัดช่วงระหว่างขอบขาขึ้นสองครั้งจะได้หนึ่งคาบเต็ม ซึ่งยาวเป็นสองเท่าของเวลาระหว่างการสลับสถานะ LED

**แบบฝึกหัด** เปลี่ยนโหมด SW0 = 1 ให้ LED กะพริบที่ 2 Hz คำนวณ half-period และจำนวน ticks ก่อนแก้โค้ด ค่าที่คาดหวังคือ 250 ms หรือประมาณ 25,000,000 ticks ที่ 100 MHz

## เอกสารอ้างอิง

- [AMD AXI Timer PG079 — Register Space](https://docs.amd.com/r/en-US/pg079-axi-timer/Register-Space)
- [AMD AXI Timer PG079 — Generate Mode](https://docs.amd.com/r/en-US/pg079-axi-timer/Generate-Mode)
- [AMD AXI Timer PG079 — Interrupts](https://docs.amd.com/r/en-US/pg079-axi-timer/Interrupts)
