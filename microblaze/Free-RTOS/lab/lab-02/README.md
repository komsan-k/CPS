# FreeRTOS LAB 02 — Task Notification บน MicroBlaze และ Nexys A7

ต่อจาก [FreeRTOS LAB 01 — สอง Task และ Queue](https://chatgpt.com/library?file_id=libfile_64ee0c3f92308191be04ac57b439bacb&account_id=personal) โดยให้ **Switch Task ส่ง Task Notification เมื่อ SW0 เปลี่ยนค่า** ส่วน **LED Task** ใช้การรอแบบมี timeout เมื่อตรวจพบการเปลี่ยนโหมด LED Task จะเริ่มนับครึ่งคาบใหม่ตามค่าที่ได้รับทันที

| SW0 | ครึ่งคาบของ LED0 | ความถี่เมื่อคงที่ |
|---|---:|---:|
| 0 | 500 ms | ประมาณ 1 Hz |
| 1 | 125 ms | ประมาณ 4 Hz |

**อุปกรณ์** Nexys A7-100T, USB-JTAG และโปรเจกต์ FreeRTOS LAB 01 ใช้ Classic MicroBlaze, AXI GPIO และ AXI Timer/INTC สำหรับ FreeRTOS tick

## 1. เตรียมโปรเจกต์

1. ใช้ **XSA, bitstream และ FreeRTOS platform จาก LAB 01** สร้าง Empty Application แยกสำหรับ LAB 02 บน FreeRTOS domain เดิม
2. ตรวจว่า FreeRTOS เปิด Task Notifications โดย configUSE_TASK_NOTIFICATIONS เป็น 1 หรือใช้ค่าเริ่มต้นของ kernel ใช้ tick rate 1,000 Hz ตาม LAB 01
3. เพิ่ม src/main.c ตามโค้ดด้านล่าง ตรวจ GPIO_BASE ให้ตรงกับ Address Editor ช่อง 1 รับ SW0 และช่อง 2 ควบคุม LED0

## 2. เขียนโปรแกรม C

```c
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "xil_io.h"

#define GPIO_BASE           0x40000000U
#define SW_DATA             (GPIO_BASE + 0x00U)
#define LED_DATA            (GPIO_BASE + 0x08U)
#define GPIO_BIT            0x01U

#define SWITCH_SAMPLE_MS    50U
#define SLOW_HALF_PERIOD_MS 500U
#define FAST_HALF_PERIOD_MS 125U
#define STACK_WORDS         256U

static TaskHandle_t led_task_handle;
static uint32_t initial_half_period_ms;

static uint32_t read_half_period_ms(void)
{
    return (Xil_In32(SW_DATA) & GPIO_BIT) == 0U ?
           SLOW_HALF_PERIOD_MS : FAST_HALF_PERIOD_MS;
}

static void switch_task(void *argument)
{
    (void)argument;
    uint32_t previous_ms = initial_half_period_ms;

    for (;;) {
        uint32_t current_ms = read_half_period_ms();

        if (current_ms != previous_ms) {
            (void)xTaskNotify(led_task_handle, current_ms,
                              eSetValueWithOverwrite);
            previous_ms = current_ms;
        }

        vTaskDelay(pdMS_TO_TICKS(SWITCH_SAMPLE_MS));
    }
}

static void led_task(void *argument)
{
    (void)argument;
    uint32_t half_period_ms = initial_half_period_ms;
    u32 led_state = 0U;

    for (;;) {
        uint32_t requested_ms;
        BaseType_t notified = xTaskNotifyWait(
            0U, 0U, &requested_ms, pdMS_TO_TICKS(half_period_ms));

        if (notified == pdTRUE) {
            half_period_ms = requested_ms;
        } else {
            led_state ^= GPIO_BIT;
            Xil_Out32(LED_DATA, led_state);
        }
    }
}

int main(void)
{
    Xil_Out32(LED_DATA, 0U);
    initial_half_period_ms = read_half_period_ms();

    if (xTaskCreate(led_task, "LED", STACK_WORDS, NULL,
                    tskIDLE_PRIORITY + 1U, &led_task_handle) != pdPASS ||
        xTaskCreate(switch_task, "Switch", STACK_WORDS, NULL,
                    tskIDLE_PRIORITY + 1U, NULL) != pdPASS) {
        return 1;
    }

    vTaskStartScheduler();
    return 1;
}
```

LED Task รอใน xTaskNotifyWait ซึ่งจบได้สองแบบ

| ผลที่ได้ | สิ่งที่ LED Task ทำ |
|---|---|
| ได้ notification | รับค่าครึ่งคาบใหม่ เริ่มรอรอบใหม่ และคงสถานะ LED ปัจจุบันระหว่างรอ |
| ครบเวลารอ | สลับสถานะ LED0 แล้วเริ่มรอครึ่งคาบถัดไป |

eSetValueWithOverwrite เก็บค่าล่าสุดให้ LED Task หากมี notification ค้างอยู่ ส่วน Switch Task ส่งเฉพาะเมื่ออ่านพบค่าต่างจากครั้งก่อน การส่งซ้ำทุก 50 ms จะทำให้ LED Task เริ่มนับครึ่งคาบใหม่ซ้ำก่อนถึง timeout

## 3. รันและสังเกตผล

โหลด bitstream ของ LAB 01 และรัน application ผ่าน Vitis/JTAG

1. ตั้ง SW0 = 0 แล้ววัดเวลาห้าคาบเต็มของ LED0 ควรได้ประมาณ 5 s
2. ตั้ง SW0 = 1 แล้ววัดห้าคาบอีกครั้ง ควรได้ประมาณ 1.25 s
3. สลับจากโหมดช้าไปโหมดเร็วระหว่างที่ LED กำลังรอครึ่งคาบ ใน LAB 01 โปรแกรมอ่านค่าใหม่หลังจบการหน่วง ส่วน LAB 02 เริ่มครึ่งคาบใหม่เมื่อได้รับ notification ภายในเวลาสุ่มอ่าน SW0 รอบถัดไป ซึ่งอยู่ที่ประมาณ 50 ms ที่การตั้งค่านี้ ให้สังเกตว่า LED ยังเปลี่ยนสถานะเมื่อครบ timeout ใหม่เท่านั้น

หาก LED0 กะพริบแต่ SW0 เปลี่ยนความถี่ไม่ได้ ให้ตรวจ GPIO_BASE, พอร์ต SW0 และว่า Switch Task ส่ง xTaskNotify โดยใช้ handle ของ LED Task หาก LED0 ค้างตั้งแต่เริ่ม ให้ตรวจการทำงานของ FreeRTOS tick ตาม LAB 01

**แบบฝึกหัด** เพิ่มความถี่การอ่าน SW0 จากทุก 50 ms เป็นทุก 10 ms แล้วเปรียบเทียบเวลาตั้งแต่สลับ SW0 จน LED Task รับค่าใหม่ โดยคงครึ่งคาบ 500 ms และ 125 ms ไว้

## เอกสารอ้างอิง

- [FreeRTOS — xTaskNotify](https://ecos.freertos.org/Documentation/02-Kernel/04-API-references/05-Direct-to-task-notifications/04-xTaskNotify)
- [Mastering the FreeRTOS Kernel — Task Notifications](https://www.freertos.org/media/2018/161204_Mastering_the_FreeRTOS_Real_Time_Kernel-A_Hands-On_Tutorial_Guide.pdf)
