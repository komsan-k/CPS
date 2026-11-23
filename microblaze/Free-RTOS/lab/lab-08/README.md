# FreeRTOS LAB 08 — รอหลายเงื่อนไขด้วย Event Group และ Timeout

ต่อจาก [FreeRTOS LAB 07 — อ่านสถานะด้วย Event Group](https://chatgpt.com/library?file_id=libfile_cde2f2baa9a881919007b8866bcb1525&account_id=personal) เพิ่มคำสั่ง `m` ให้ **Monitor Task** รอจนสองบิตเป็น 1 พร้อมกัน ได้แก่ LED0 อยู่ในโหมด **RUNNING** และเลือกความถี่ **4 Hz** หากยังไม่ครบเงื่อนไขเมื่อเวลาผ่านไป 5 วินาที โปรแกรมรายงาน **MONITOR TIMEOUT** ผ่าน UART

**อุปกรณ์** Nexys A7-100T, USB-JTAG และ XSA/bitstream จาก LAB 07 ใช้ Classic MicroBlaze และ FreeRTOS domain เดิม

| คำสั่ง | การทำงาน |
|---|---|
| `m` | เริ่มตรวจเงื่อนไขหนึ่งครั้ง รอสูงสุด 5 s |
| `r` | อ่านสถานะปัจจุบันทันที |
| `p` / `g` | พัก / กลับมากะพริบ LED0 |
| `?` | ดูรายการคำสั่ง |

## 1. เตรียม Application ใน Vitis

ใช้ hardware platform และ FreeRTOS BSP จาก LAB 07 สร้าง Empty Application ใหม่ ตรวจว่า build Event Groups, Software Timers และ Task Notifications แล้ว และใช้ tick rate 1,000 Hz ตาม LAB เดิม ตรวจ AXI GPIO และ AXI UART Lite base address ก่อนเพิ่มโค้ดลงใน `src/main.c`

Monitor Task ใช้ `xEventGroupWaitBits()` ตั้ง **รอครบทุกบิต** และ **เก็บบิตเดิมหลังตื่น** ผลที่คืนมาบอกว่าครบเงื่อนไขหรือหมดเวลารอ:

| RUNNING | FAST | ผลของ Monitor |
|---:|---:|---|
| 0 | 0 | รอต่อหรือ timeout |
| 0 | 1 | รอต่อหรือ timeout |
| 1 | 0 | รอต่อหรือ timeout |
| 1 | 1 | ส่ง MONITOR READY |

บิต FAST หมายถึงความถี่ที่ Switch Task เลือกไว้สำหรับ Software Timer การทดลองจึงตรวจเงื่อนไขของระบบจากบิตสถานะ ส่วนการวัดความถี่จริงทำได้จาก LED0 ตาม LAB ก่อนหน้า

## 2. โปรแกรม C

```c
#include <stdint.h>

#include "FreeRTOS.h"
#include "event_groups.h"
#include "queue.h"
#include "task.h"
#include "timers.h"
#include "xil_io.h"

#define GPIO_BASE           0x40000000U
#define UART_BASE           0x40600000U
#define UART_RX             (UART_BASE + 0x00U)
#define UART_TX             (UART_BASE + 0x04U)
#define UART_STATUS         (UART_BASE + 0x08U)
#define UART_RX_VALID       (1U << 0)
#define UART_TX_FULL        (1U << 3)
#define LOG_QUEUE_LENGTH    8U
#define SW_DATA             (GPIO_BASE + 0x00U)
#define LED_DATA            (GPIO_BASE + 0x08U)
#define GPIO_BIT            0x01U
#define BUTTON_BIT          0x02U
#define EVENT_BLINK         (1UL << 0)
#define EVENT_BUTTON        (1UL << 1)
#define EVENT_SERIAL        (1UL << 2)
#define STATUS_RUNNING      (1U << 0)
#define STATUS_FAST         (1U << 1)
#define MONITOR_TIMEOUT_MS  5000U
#define UART_POLL_MS        5U
#define BUTTON_SAMPLE_MS    10U
#define BUTTON_SAMPLES      3U

#define SWITCH_SAMPLE_MS    50U
#define SLOW_HALF_PERIOD_MS 500U
#define FAST_HALF_PERIOD_MS 125U
#define STACK_WORDS         256U

enum log_event {
    LOG_PAUSED, LOG_RESUMED, LOG_SLOW, LOG_FAST,
    LOG_MONITOR_READY, LOG_MONITOR_TIMEOUT
};

static QueueHandle_t log_queue;
static QueueHandle_t command_queue;
static EventGroupHandle_t system_status;
static TaskHandle_t led_task_handle;
static TaskHandle_t monitor_task_handle;
static TimerHandle_t blink_timer;
static TickType_t initial_period;

static TickType_t read_blink_period(void)
{
    return (Xil_In32(SW_DATA) & GPIO_BIT) == 0U ?
           pdMS_TO_TICKS(SLOW_HALF_PERIOD_MS) :
           pdMS_TO_TICKS(FAST_HALF_PERIOD_MS);
}

static void post_log(enum log_event event)
{
    (void)xQueueSend(log_queue, &event, 0U);
}

static void publish_rate(TickType_t period)
{
    if (period == pdMS_TO_TICKS(FAST_HALF_PERIOD_MS)) {
        (void)xEventGroupSetBits(system_status, STATUS_FAST);
        post_log(LOG_FAST);
    } else {
        (void)xEventGroupClearBits(system_status, STATUS_FAST);
        post_log(LOG_SLOW);
    }
}

static void uart_print(const char *message)
{
    while (*message != '\0') {
        while ((Xil_In32(UART_STATUS) & UART_TX_FULL) != 0U) {
            vTaskDelay(1U);
        }
        Xil_Out32(UART_TX, (unsigned char)*message);
        ++message;
    }
}

static void print_status(void)
{
    EventBits_t bits = xEventGroupGetBits(system_status);
    uart_print((bits & STATUS_RUNNING) != 0U ? "MODE=RUNNING" : "MODE=PAUSED");
    uart_print((bits & STATUS_FAST) != 0U ? " SELECTED=4 Hz\r\n" : " SELECTED=1 Hz\r\n");
}

static void monitor_task(void *argument)
{
    (void)argument;
    const EventBits_t target = STATUS_RUNNING | STATUS_FAST;

    for (;;) {
        if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) == 0U) {
            continue;
        }

        EventBits_t result = xEventGroupWaitBits(
            system_status, target, pdFALSE, pdTRUE,
            pdMS_TO_TICKS(MONITOR_TIMEOUT_MS));

        post_log((result & target) == target ?
                 LOG_MONITOR_READY : LOG_MONITOR_TIMEOUT);
    }
}

static void handle_serial(unsigned char command)
{
    if (command == 'p' || command == 'g') {
        u32 requested_pause = (command == 'p') ? 1U : 0U;
        (void)xQueueOverwrite(command_queue, &requested_pause);
        (void)xTaskNotify(led_task_handle, EVENT_SERIAL, eSetBits);
    } else if (command == 'r') {
        print_status();
    } else if (command == 'm') {
        (void)xTaskNotifyGive(monitor_task_handle);
        uart_print("MONITOR ARMED\r\n");
    } else if (command == '?') {
        uart_print("p: pause | g: go | r: status | m: monitor | BTNU: toggle | SW0: rate\r\n");
    } else if (command != '\r' && command != '\n') {
        uart_print("Unknown command. Send ?\r\n");
    }
}

static void uart_task(void *argument)
{
    (void)argument;
    static const char *const messages[] = {
        "LED PAUSED\r\n", "LED RESUMED\r\n", "RATE 1 Hz\r\n", "RATE 4 Hz\r\n",
        "MONITOR READY: RUNNING + 4 Hz selected\r\n", "MONITOR TIMEOUT\r\n"
    };

    uart_print("FreeRTOS LAB 08 ready. Send ? for help.\r\n");
    for (;;) {
        if ((Xil_In32(UART_STATUS) & UART_RX_VALID) != 0U) {
            handle_serial((unsigned char)Xil_In32(UART_RX));
        }

        enum log_event event;
        if (xQueueReceive(log_queue, &event, pdMS_TO_TICKS(UART_POLL_MS)) == pdPASS) {
            uart_print(messages[event]);
        }
    }
}

static void blink_callback(TimerHandle_t timer)
{
    (void)timer;
    (void)xTaskNotify(led_task_handle, EVENT_BLINK, eSetBits);
}

static void button_callback(TimerHandle_t timer)
{
    (void)timer;
    static u32 stable;
    static unsigned int different_samples;
    u32 raw = Xil_In32(SW_DATA) & BUTTON_BIT;

    if (raw == stable) {
        different_samples = 0U;
    } else if (++different_samples == BUTTON_SAMPLES) {
        different_samples = 0U;
        stable = raw;
        if (stable != 0U) {
            (void)xTaskNotify(led_task_handle, EVENT_BUTTON, eSetBits);
        }
    }
}

static void led_task(void *argument)
{
    (void)argument;
    u32 led_state = 0U;
    u32 paused = 0U;

    for (;;) {
        uint32_t events;
        if (xTaskNotifyWait(0U, UINT32_MAX, &events, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        u32 requested_pause = paused;
        if ((events & EVENT_BUTTON) != 0U) {
            requested_pause ^= 1U;
        }
        if ((events & EVENT_SERIAL) != 0U) {
            (void)xQueueReceive(command_queue, &requested_pause, 0U);
        }

        if (requested_pause != paused) {
            paused = requested_pause;
            led_state = 0U;
            Xil_Out32(LED_DATA, led_state);
            if (paused != 0U) {
                (void)xEventGroupClearBits(system_status, STATUS_RUNNING);
                post_log(LOG_PAUSED);
            } else {
                (void)xEventGroupSetBits(system_status, STATUS_RUNNING);
                post_log(LOG_RESUMED);
            }
        } else if (paused == 0U && (events & EVENT_BLINK) != 0U) {
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
            publish_rate(period);
        }

        vTaskDelay(pdMS_TO_TICKS(SWITCH_SAMPLE_MS));
    }
}

int main(void)
{
    Xil_Out32(LED_DATA, 0U);
    log_queue = xQueueCreate(LOG_QUEUE_LENGTH, sizeof(enum log_event));
    command_queue = xQueueCreate(1U, sizeof(u32));
    system_status = xEventGroupCreate();
    if (log_queue == NULL || command_queue == NULL || system_status == NULL) {
        return 1;
    }

    initial_period = read_blink_period();
    (void)xEventGroupSetBits(system_status, STATUS_RUNNING);
    publish_rate(initial_period);
    blink_timer = xTimerCreate("Blink", initial_period, pdTRUE,
                               NULL, blink_callback);

    TimerHandle_t button_timer = xTimerCreate(
        "Button", pdMS_TO_TICKS(BUTTON_SAMPLE_MS), pdTRUE, NULL, button_callback);

    if (blink_timer == NULL || button_timer == NULL) {
        return 1;
    }

    if (xTaskCreate(led_task, "LED", STACK_WORDS, NULL,
                    tskIDLE_PRIORITY + 1U, &led_task_handle) != pdPASS ||
        xTaskCreate(switch_task, "Switch", STACK_WORDS, NULL,
                    tskIDLE_PRIORITY + 1U, NULL) != pdPASS ||
        xTaskCreate(uart_task, "UART", STACK_WORDS, NULL,
                    tskIDLE_PRIORITY + 1U, NULL) != pdPASS ||
        xTaskCreate(monitor_task, "Monitor", STACK_WORDS, NULL,
                    tskIDLE_PRIORITY + 1U, &monitor_task_handle) != pdPASS) {
        return 1;
    }

    if (xTimerStart(blink_timer, 0U) != pdPASS ||
        xTimerStart(button_timer, 0U) != pdPASS) {
        return 1;
    }

    vTaskStartScheduler();
    return 1;
}
```

UART Task ส่ง notification เมื่อรับ `m` แล้วกลับไปรับคำสั่งตามปกติ ขณะที่ Monitor Task อยู่ในสถานะ Blocked เมื่อรอ Event Group ให้ส่ง `m` ทีละครั้งและรอผลก่อนเริ่มการตรวจใหม่ เพื่อให้จับคู่ผลกับการทดลองได้ง่าย

## 3. รันและทดสอบ

เปิด Serial Terminal ที่ **115200 baud, 8-N-1, flow control = None** แล้วโหลด bitstream และรัน LAB 08 ผ่าน USB-JTAG

| กรณี | ขั้นตอน | ผลที่คาดหวัง |
|---|---|---|
| ครบเงื่อนไขตั้งแต่เริ่ม | ตั้ง SW0 = 1 ส่ง `g` แล้วส่ง `m` | ได้ MONITOR READY โดยไม่ต้องรอ timeout |
| ครบเงื่อนไขภายหลัง | ตั้ง SW0 = 1 ส่ง `p` รอให้พัก ส่ง `m` แล้วส่ง `g` ภายใน 5 s | ได้ MONITOR READY หลังกลับมากะพริบ |
| ครบเพียงบิตเดียว | ตั้ง SW0 = 0 ส่ง `g` แล้วส่ง `m` และคง SW0 ไว้ | ได้ MONITOR TIMEOUT หลังประมาณ 5 s |
| อ่านสถานะระหว่างรอ | ระหว่างกรณี timeout ให้ส่ง `r` | เห็นสถานะทันที แล้วจึงเห็น MONITOR TIMEOUT เมื่อครบเวลา |
| ตรวจว่าบิตยังอยู่ | หลัง MONITOR READY ให้ส่ง `r` | เห็น `MODE=RUNNING SELECTED=4 Hz` หากอินพุตคงเดิม |

**แบบฝึกหัด** เปลี่ยนพารามิเตอร์ wait-for-all จาก `pdTRUE` เป็น `pdFALSE` แล้วปรับเงื่อนไขตรวจค่าที่คืนมาให้สำเร็จเมื่อมีอย่างน้อยหนึ่งบิต จากนั้นทดสอบกรณีที่ LED0 กำลังกะพริบที่ 1 Hz และอธิบายผลที่ต่างจากเงื่อนไขเดิม

## เอกสารอ้างอิง

- [FreeRTOS — xEventGroupWaitBits](https://freertos.org/Documentation/02-Kernel/04-API-references/12-Event-groups-or-flags/04-xEventGroupWaitBits)
- [FreeRTOS Reference Manual](https://www.freertos.org/media/2018/FreeRTOS_Reference_Manual_V10.0.0.pdf)
