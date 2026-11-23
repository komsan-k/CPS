# FreeRTOS LAB 03 — Software Timer บน MicroBlaze และ Nexys A7

ต่อจาก [FreeRTOS LAB 02 — Task Notification](https://chatgpt.com/library?file_id=libfile_6e77741db83481918734b1104d35342d&account_id=personal) โดยใช้ **FreeRTOS Software Timer แบบ auto-reload** กำหนดเวลาระหว่างการสลับ LED0 เมื่อครบเวลา callback จะส่ง notification ให้ LED Task ส่วน Switch Task อ่าน SW0 และเปลี่ยนคาบของ timer

| SW0 | คาบของ Software Timer | คาบเต็มของ LED0 | ความถี่ LED0 |
|---|---:|---:|---:|
| 0 | 500 ms | 1 s | ประมาณ 1 Hz |
| 1 | 125 ms | 250 ms | ประมาณ 4 Hz |

**อุปกรณ์** Nexys A7-100T, USB-JTAG และโปรเจกต์ FreeRTOS LAB 02 ใช้ Classic MicroBlaze, AXI GPIO และ AXI Timer/INTC ที่เป็นแหล่ง FreeRTOS tick

## 1. เตรียมโปรเจกต์

1. ใช้ **XSA และ bitstream จาก LAB 02** และสร้าง Empty Application บน FreeRTOS domain เดิม
2. ตรวจการตั้งค่า FreeRTOS ใน BSP ให้เปิด software timers (configUSE_TIMERS = 1) และ task notifications (configUSE_TASK_NOTIFICATIONS = 1 หรือค่าเริ่มต้นของ kernel) ใช้ tick rate 1,000 Hz ตาม LAB 02 ค่าแนะนำสำหรับการทดลองคือ timer task priority 2, timer command queue length อย่างน้อย 4 และ timer task stack depth 256 words หากเปลี่ยน BSP ให้ build platform ใหม่
3. ตรวจ GPIO_BASE ให้ตรงกับ Address Editor แล้วใส่โค้ดใน src/main.c ช่อง 1 ของ AXI GPIO รับ SW0 และช่อง 2 ควบคุม LED0

AXI Timer สร้าง kernel tick ให้ FreeRTOS ส่วน Software Timer นับเวลาจาก tick เหล่านี้แล้วเรียก callback ใน **Timer Service Task** โปรแกรมนี้จึงใช้ AXI Timer ตัวเดิมและเพิ่ม Software Timer ในเฟิร์มแวร์

## 2. โปรแกรม C

```c
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
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
static TimerHandle_t blink_timer;
static TickType_t initial_period;

static TickType_t read_blink_period(void)
{
    return (Xil_In32(SW_DATA) & GPIO_BIT) == 0U ?
           pdMS_TO_TICKS(SLOW_HALF_PERIOD_MS) :
           pdMS_TO_TICKS(FAST_HALF_PERIOD_MS);
}

static void blink_callback(TimerHandle_t timer)
{
    (void)timer;
    (void)xTaskNotifyGive(led_task_handle);
}

static void led_task(void *argument)
{
    (void)argument;
    u32 led_state = 0U;

    for (;;) {
        if (ulTaskNotifyTake(pdFALSE, portMAX_DELAY) != 0U) {
            led_state ^= GPIO_BIT;
            Xil_Out32(LED_DATA, led_state);
        }
    }
}

static void switch_task(void *argument)
{
    (void)argument;
    TickType_t previous_period = initial_period;

    for (;;) {
        TickType_t period = read_blink_period();
        if (period != previous_period &&
            xTimerChangePeriod(blink_timer, period, 0U) == pdPASS) {
            previous_period = period;
        }

        vTaskDelay(pdMS_TO_TICKS(SWITCH_SAMPLE_MS));
    }
}

int main(void)
{
    Xil_Out32(LED_DATA, 0U);
    initial_period = read_blink_period();
    blink_timer = xTimerCreate("Blink", initial_period, pdTRUE,
                               NULL, blink_callback);

    if (blink_timer == NULL) {
        return 1;
    }

    if (xTaskCreate(led_task, "LED", STACK_WORDS, NULL,
                    tskIDLE_PRIORITY + 1U, &led_task_handle) != pdPASS ||
        xTaskCreate(switch_task, "Switch", STACK_WORDS, NULL,
                    tskIDLE_PRIORITY + 1U, NULL) != pdPASS) {
        return 1;
    }

    if (xTimerStart(blink_timer, 0U) != pdPASS) {
        return 1;
    }

    vTaskStartScheduler();
    return 1;
}
```

Callback ทำงานในบริบทของ task และส่ง notification แล้วคืนการควบคุมทันที LED Task รอด้วย ulTaskNotifyTake โดย pdFALSE ทำให้ลดตัวนับ notification ทีละหนึ่งตามจำนวนเหตุการณ์ที่รับมา Switch Task ส่งคำสั่งเปลี่ยนคาบเมื่อค่าต่างจากครั้งก่อน หากคิวคำสั่ง timer เต็ม โปรแกรมจะลองใหม่ในการอ่าน SW0 รอบถัดไป

## 3. รันและทดสอบ

1. โหลด bitstream เดิมและรัน application ของ LAB 03 ผ่าน USB-JTAG
2. ตั้ง SW0 = 0 ให้ LED0 กะพริบคงที่ แล้ววัดจากขอบขาขึ้นครั้งแรกถึงครั้งที่หก ควรได้ประมาณ 5 s ทำซ้ำเมื่อ SW0 = 1 ควรได้ประมาณ 1.25 s
3. สลับ SW0 ขณะ LED0 กำลังกะพริบ แล้วสังเกตการเปลี่ยนจังหวะ หากต้องการดูเส้นทางการทำงาน ให้ตั้ง breakpoint ที่ blink_callback และ LED Task แล้วกด Resume เอา breakpoint ออกก่อนวัดคาบ

หาก LED0 ค้าง ให้ตรวจว่าใช้ FreeRTOS domain, เปิด software timers แล้ว และมี kernel tick จาก AXI Timer หากโปรแกรมกลับจาก vTaskStartScheduler ให้ตรวจพื้นที่ heap สำหรับ tasks, software timer และ Timer Service Task

**แบบฝึกหัด** ปรับโหมด SW0 = 1 ให้เป็น 2 Hz แล้วตรวจว่าห้าคาบเต็มใช้ประมาณ 2.5 s อธิบายว่าทำไมต้องตั้ง Software Timer ที่ 250 ms เพื่อให้ LED กะพริบคาบเต็ม 500 ms

## เอกสารอ้างอิง

- [FreeRTOS Reference Manual — Software Timers and Task Notifications](https://www.freertos.org/media/2018/FreeRTOS_Reference_Manual_V10.0.0.pdf)
- [Mastering the FreeRTOS Kernel — Software Timers](https://www.freertos.org/media/2018/161204_Mastering_the_FreeRTOS_Real_Time_Kernel-A_Hands-On_Tutorial_Guide.pdf)
