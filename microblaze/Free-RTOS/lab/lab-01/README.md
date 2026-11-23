
# FreeRTOS LAB 01 — สอง Task บน MicroBlaze และ Nexys A7

ใน LAB นี้ **Switch Task** อ่าน SW0 ทุก 50 ms แล้วส่งค่าล่าสุดผ่าน FreeRTOS Queue ให้ **LED Task** เลือกความถี่กะพริบของ LED0 นักศึกษาจะได้ใช้ task, queue และ scheduler บน Classic MicroBlaze โดยใช้อุปกรณ์บนบอร์ด Nexys A7-100T

| SW0 | เวลาที่ LED0 อยู่ในแต่ละสถานะ | ความถี่ที่คาดหวัง |
|---|---:|---:|
| 0 | 500 ms | ≈ 1 Hz |
| 1 | 125 ms | ≈ 4 Hz |

**อุปกรณ์** Nexys A7-100T, สาย USB-JTAG และ Vivado กับ Vitis Unified IDE รุ่นเดียวกัน

## 1. เตรียมฮาร์ดแวร์ใน Vivado

ใช้สำเนา Block Design ของ [MicroBlaze LAB 03 — AXI Timer Interrupt](https://chatgpt.com/library?file_id=libfile_528bf865e7a881918bfb6d7521d31755&account_id=personal) หากเริ่มจาก [LAB 01 — AXI GPIO](https://chatgpt.com/library?file_id=libfile_e92c6ff5e4048191801f7c73ab481da3&account_id=personal) ให้เพิ่ม AXI Timer และ AXI Interrupt Controller แล้วต่อสัญญาณตามตาราง

| ส่วนประกอบ | การตั้งค่าที่ใช้ |
|---|---|
| Classic MicroBlaze | Clock 100 MHz, JTAG debug module และ local BRAM แนะนำ 128 KiB สำหรับการสร้างใหม่เพื่อเผื่อพื้นที่โค้ดและ heap |
| AXI GPIO ช่อง 1 | อินพุต 1 บิตจาก SW0 ขา J15 |
| AXI GPIO ช่อง 2 | เอาต์พุต 1 บิตไป LED0 ขา H17 |
| AXI Timer | เชื่อม S_AXI กับ AXI interconnect ใช้ Timer 0 ขนาด 32 บิตและ clock 100 MHz |
| AXI INTC | AXI Timer interrupt → intr[0] แบบ active-high level แล้วต่อ INTC interrupt → MicroBlaze INTERRUPT |

ตรวจ AXI clock และ peripheral reset ของทุกบล็อก ใช้ XDC จาก LAB 01 หรือ LAB 03 และตรวจชื่อพอร์ตให้ตรงกับ wrapper เปิด Address Editor เพื่อตรวจ base address ของ AXI GPIO ตัวอย่างใช้ 0x40000000 จากนั้น Validate Design สร้าง bitstream และ Export Hardware เป็น XSA พร้อม Include bitstream

FreeRTOS ใช้ AXI Timer เป็นแหล่ง kernel tick ผ่าน AXI INTC โปรแกรมตัวอย่างจึงปล่อยให้ FreeRTOS port และ BSP ตั้งค่า Timer และเส้นทาง interrupt

## 2. สร้าง FreeRTOS domain ใน Vitis

1. สร้าง **Platform Component** จาก XSA แล้วเลือก Classic MicroBlaze เช่น microblaze_0 และเลือก OS **FreeRTOS** บางรุ่นแสดงชื่อ freertos10_xilinx หากมี platform เดิม ให้ใช้ Add Domain เพื่อเพิ่ม FreeRTOS domain ได้
2. ตรวจ FreeRTOS และ BSP settings ใช้ tick rate **1,000 Hz** ตามตัวอย่าง หาก BSP แสดงตัวเลือก tick หรือ interval timer ให้เลือก AXI Timer ที่ต่อกับ INTC ในโปรเจกต์นี้ ตรวจว่าเปิด dynamic allocation สำหรับ xQueueCreate และ xTaskCreate
3. Build platform จากนั้นสร้าง **Empty Application** ใน FreeRTOS domain และเพิ่ม src/main.c ตามโค้ดด้านล่าง หาก AXI GPIO ใช้ base address อื่น ให้แก้ GPIO_BASE ให้ตรงกับ Address Editor

## 3. โค้ด C

```c
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "xil_io.h"

#define GPIO_BASE     0x40000000U
#define SW_DATA       (GPIO_BASE + 0x00U)
#define LED_DATA      (GPIO_BASE + 0x08U)
#define GPIO_BIT      0x01U
#define STACK_WORDS   256U

static QueueHandle_t rate_queue;

static u32 read_half_period_ms(void)
{
    return (Xil_In32(SW_DATA) & GPIO_BIT) == 0U ? 500U : 125U;
}

static void switch_task(void *argument)
{
    (void)argument;

    for (;;) {
        u32 half_period_ms = read_half_period_ms();
        xQueueOverwrite(rate_queue, &half_period_ms);
        vTaskDelay(pdMS_TO_TICKS(50U));
    }
}

static void led_task(void *argument)
{
    (void)argument;
    u32 led = 0U;
    u32 half_period_ms = 500U;

    for (;;) {
        (void)xQueuePeek(rate_queue, &half_period_ms, 0U);
        led ^= GPIO_BIT;
        Xil_Out32(LED_DATA, led);
        vTaskDelay(pdMS_TO_TICKS(half_period_ms));
    }
}

int main(void)
{
    Xil_Out32(LED_DATA, 0U);

    rate_queue = xQueueCreate(1U, sizeof(u32));
    if (rate_queue == NULL) {
        return 1;
    }

    u32 initial_half_period_ms = read_half_period_ms();
    xQueueOverwrite(rate_queue, &initial_half_period_ms);

    if (xTaskCreate(switch_task, "Switch", STACK_WORDS, NULL,
                    tskIDLE_PRIORITY + 1U, NULL) != pdPASS ||
        xTaskCreate(led_task, "LED", STACK_WORDS, NULL,
                    tskIDLE_PRIORITY + 1U, NULL) != pdPASS) {
        return 1;
    }

    vTaskStartScheduler();
    return 1;   /* The scheduler should not return during normal operation. */
}
```

คิวหนึ่งช่องเก็บค่าล่าสุดด้วย xQueueOverwrite ส่วน xQueuePeek ให้ LED Task อ่านค่าโดยเก็บค่าไว้ในคิว vTaskDelay ทำให้ task เข้าสู่สถานะ Blocked ระหว่างรอ Scheduler จึงสามารถให้ task อื่นทำงานได้ เมื่อเปลี่ยน SW0 ค่าใหม่จะถูกใช้ตอนเริ่มช่วงติดหรือดับถัดไป

## 4. รันและตรวจผล

1. ต่อ USB-JTAG เปิดบอร์ดแล้ว build และรัน application บนฮาร์ดแวร์ หาก debugger หยุดที่ main ให้กด Resume
2. ตั้ง SW0 = 0 รอให้จังหวะคงที่ แล้ววัดเวลาห้าคาบจากขอบขาขึ้นครั้งแรกถึงครั้งที่หก ควรได้ประมาณ 5 s เปลี่ยน SW0 = 1 แล้ววัดซ้ำ ควรได้ประมาณ 1.25 s
3. หาก LED0 ค้างหลังสลับครั้งแรก ให้ตรวจเส้น AXI Timer → AXI INTC → MicroBlaze และ FreeRTOS domain หาก xTaskCreate ล้มเหลวหรือ scheduler กลับมาที่ main ให้ตรวจพื้นที่ BRAM ใน linker script และ FreeRTOS heap ใน BSP

**แบบฝึกหัด** เปลี่ยนความถี่เมื่อ SW0 = 1 ให้เป็น 2 Hz โดยแก้เฉพาะค่าครึ่งคาบใน read_half_period_ms ค่าที่คาดหวังคือ 250 ms ต่อครึ่งคาบ หรือประมาณ 2.5 s สำหรับห้าคาบเต็ม

## เอกสารอ้างอิง

- [AMD FreeRTOS — Hardware platform requirements and Vitis setup](https://xilinx-wiki.atlassian.net/wiki/spaces/A/pages/18842141)
- [AMD Vitis UG1400 — Adding a domain](https://docs.amd.com/r/en-US/ug1400-vitis-embedded/Adding-a-Domain-to-an-Existing-Platform)
- [FreeRTOS Reference Manual](https://freertos.org/media/2025/FreeRTOS_Reference_Manual_V8.2.1.pdf)
