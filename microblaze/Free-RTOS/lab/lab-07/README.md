# FreeRTOS LAB 07 — อ่านสถานะระบบด้วย Event Group บน MicroBlaze

ต่อจาก [FreeRTOS LAB 06 — ควบคุม LED0 จาก Serial Terminal](https://chatgpt.com/library?file_id=libfile_6ce3d6cf2154819185bf73508d6f2d35&account_id=personal) เพิ่มคำสั่ง **r** ให้ UART Task อ่านสถานะการกะพริบและความถี่ที่เลือกจาก **FreeRTOS Event Group** ตัวอย่างเมื่อพัก LED0 แต่เลือกโหมดเร็วอยู่

```text
MODE=PAUSED SELECTED=4 Hz
```

**อุปกรณ์** Nexys A7-100T, USB-JTAG และ XSA/bitstream จาก LAB 06 ใช้ Classic MicroBlaze และ FreeRTOS domain เดิม

| บิต | Task ที่อัปเดตขณะรัน | ค่า 0 | ค่า 1 |
|---|---|---|---|
| `STATUS_RUNNING` | LED Task | พักการกะพริบ | กำลังทำงานในโหมดกะพริบ |
| `STATUS_FAST` | Switch Task | เลือก 1 Hz | เลือก 4 Hz |

`MODE` คือโหมดการทำงานของ LED0 ส่วน `SELECTED` คือความถี่ที่เลือกไว้สำหรับตอนกะพริบ โดย Switch Task อัปเดตบิตหลัง FreeRTOS รับคำสั่งตั้งคาบเข้าคิวของ Timer Service แล้ว UART Task อ่านทั้งสองบิตด้วย `xEventGroupGetBits()` ครั้งเดียว

## 1. เตรียมโปรเจกต์

1. สร้าง Empty Application ใหม่บน platform ของ LAB 06 ใช้ FreeRTOS domain และ tick rate 1,000 Hz เดิม เปิด dynamic allocation, Software Timers และ Task Notifications
2. ตรวจว่า BSP มี `event_groups.h` และ build kernel source `event_groups.c` หาก linker หา `xEventGroupCreate` ไม่พบ ให้ตรวจทั้ง FreeRTOS domain และการ build BSP ก่อน
3. เพิ่มโค้ดด้านล่างเป็น `src/main.c` ตรวจ base address ของ AXI GPIO และ AXI UART Lite ตาม Address Editor หากใช้ XSA ที่มีค่าแตกต่างจากตัวอย่าง

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
#define UART_POLL_MS        5U
#define BUTTON_SAMPLE_MS    10U
#define BUTTON_SAMPLES      3U

#define SWITCH_SAMPLE_MS    50U
#define SLOW_HALF_PERIOD_MS 500U
#define FAST_HALF_PERIOD_MS 125U
#define STACK_WORDS         256U

enum log_event {
    LOG_PAUSED, LOG_RESUMED, LOG_SLOW, LOG_FAST
};

static QueueHandle_t log_queue;
static QueueHandle_t command_queue;
static EventGroupHandle_t system_status;
static TaskHandle_t led_task_handle;
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

static void handle_serial(unsigned char command)
{
    if (command == 'p' || command == 'g') {
        u32 requested_pause = (command == 'p') ? 1U : 0U;
        (void)xQueueOverwrite(command_queue, &requested_pause);
        (void)xTaskNotify(led_task_handle, EVENT_SERIAL, eSetBits);
    } else if (command == 'r') {
        print_status();
    } else if (command == '?') {
        uart_print("p: pause | g: go | r: status | BTNU: toggle | SW0: rate\r\n");
    } else if (command != '\r' && command != '\n') {
        uart_print("Unknown command. Send ?\r\n");
    }
}

static void uart_task(void *argument)
{
    (void)argument;
    static const char *const messages[] = {
        "LED PAUSED\r\n", "LED RESUMED\r\n", "RATE 1 Hz\r\n", "RATE 4 Hz\r\n"
    };

    uart_print("FreeRTOS LAB 07 ready. Send ? for help.\r\n");
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
                    tskIDLE_PRIORITY + 1U, NULL) != pdPASS) {
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

Task Notification ยังใช้ปลุก LED Task เมื่อ timer ครบเวลา หรือเมื่อได้รับคำสั่งจาก UART และ BTNU ส่วน Event Group เก็บค่าที่ task อื่นอ่านซ้ำได้ตามต้องการ คำสั่ง `r` จึงเปิดดูสถานะหลายครั้งได้โดยไม่ล้างบิต

## 3. ทดสอบบน Nexys A7

เปิด Serial Terminal ที่ **115200 baud, 8-N-1, flow control = None** โหลด bitstream เดิมแล้วรัน application LAB 07 ผ่าน JTAG ส่งคำสั่งครั้งละหนึ่งตัว และรอข้อความยืนยันก่อนสอบถามสถานะถัดไป

| ขั้นตอน | ผลที่คาดหวังเมื่อส่ง `r` |
|---|---|
| เริ่มด้วย SW0 = 0 | `MODE=RUNNING SELECTED=1 Hz` |
| ส่ง `p` และรอ `LED PAUSED` | `MODE=PAUSED SELECTED=1 Hz` |
| ขณะพัก เปลี่ยน SW0 = 1 และรอ `RATE 4 Hz` | `MODE=PAUSED SELECTED=4 Hz` |
| ส่ง `g` และรอ `LED RESUMED` | `MODE=RUNNING SELECTED=4 Hz` |
| กด BTNU หนึ่งครั้งและรอ `LED PAUSED` | `MODE=PAUSED SELECTED=4 Hz` |

**แบบฝึกหัด** เพิ่ม task ที่ใช้ `xEventGroupWaitBits()` รอให้ระบบอยู่ในโหมดกะพริบเร็ว โดยใช้ mask `STATUS_RUNNING | STATUS_FAST` เลือกรอให้ครบทั้งสองบิต และตั้ง clear-on-exit เป็น false แล้วส่งข้อความเข้าคิวรายงานเพียงครั้งเดียวก่อนพัก task ไว้หรือจบ task ตรวจว่าเมื่อเลือก 4 Hz ระหว่างพัก task จะตื่นหลังสั่งให้กลับมากะพริบ

## เอกสารอ้างอิง

- [FreeRTOS — xEventGroupCreate](https://www.freertos.org/xEventGroupCreate.html)
- [FreeRTOS Reference Manual — Event Groups](https://www.freertos.org/media/2018/FreeRTOS_Reference_Manual_V10.0.0.pdf)
