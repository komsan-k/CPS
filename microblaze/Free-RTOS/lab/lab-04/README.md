# FreeRTOS LAB 04 — Debounce ปุ่ม BTNU ด้วย Software Timer

ต่อจาก [FreeRTOS LAB 03 — Software Timer](https://chatgpt.com/library?file_id=libfile_a81fa8957cdc8191a2956121306732ba&account_id=personal) เพิ่มปุ่ม **BTNU** สำหรับหยุดและกลับมากะพริบ LED0 ส่วน **SW0** ยังเลือกความถี่ 1 Hz หรือ 4 Hz ตามเดิม ระบบอ่านปุ่มทุก 10 ms และยอมรับสถานะใหม่เมื่ออ่านค่าเดียวกันติดต่อกัน 3 ครั้ง

**อุปกรณ์** Nexys A7-100T, USB-JTAG และโปรเจกต์ FreeRTOS LAB 03 ใช้ Classic MicroBlaze, AXI GPIO และ AXI Timer/INTC สำหรับ FreeRTOS tick

## 1. เพิ่ม BTNU ใน Vivado

1. เปิดสำเนา Block Design ของ LAB 03 เปลี่ยน AXI GPIO Channel 1 เป็น **All Inputs, Width = 2** เก็บ Channel 2 เป็นเอาต์พุต LED0 ขนาด 1 บิต
2. เพิ่ม **Concat (xlconcat)** ที่มีอินพุต 1 บิตสองขา ต่อ SW0 เดิมเข้าที่ In0 และสร้าง external input ชื่อ `btnu` ต่อเข้า In1 ต่อ `dout[1:0]` เข้าช่อง 1 ของ AXI GPIO

| สัญญาณ | GPIO ช่อง 1 | ขา FPGA |
|---|---|---|
| SW0 | บิต 0 | J15 |
| BTNU | บิต 1 | M18 |

3. เพิ่มบรรทัดต่อไปนี้ใน XDC ตรวจให้ชื่อพอร์ตตรงกับ HDL wrapper หากใช้ Board Interface ที่มีการผูกขา BTNU แล้ว ให้ใช้ constraint เพียงชุดเดียว

```tcl
set_property -dict {PACKAGE_PIN M18 IOSTANDARD LVCMOS33} [get_ports {btnu}]
```

4. ใช้เส้นทาง AXI Timer → AXI INTC → MicroBlaze จาก LAB 03 แล้ว Validate Design, Generate Bitstream และ Export Hardware พร้อม Include bitstream

## 2. โปรแกรม FreeRTOS

สร้าง Platform Component จาก XSA ของ LAB 04 บน FreeRTOS domain แล้วใช้ tick rate 1,000 Hz พร้อมเปิด Software Timers และ Task Notifications เหมือน LAB 03 สร้าง Empty Application แล้วเพิ่มโค้ดเป็น `src/main.c` ตรวจ `GPIO_BASE` ให้ตรงกับ Address Editor

```c
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "xil_io.h"

#define GPIO_BASE           0x40000000U
#define SW_DATA             (GPIO_BASE + 0x00U)
#define LED_DATA            (GPIO_BASE + 0x08U)
#define GPIO_BIT            0x01U
#define BUTTON_BIT          0x02U
#define EVENT_BLINK         (1UL << 0)
#define EVENT_BUTTON        (1UL << 1)
#define BUTTON_SAMPLE_MS    10U
#define BUTTON_SAMPLES      3U

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

        if ((events & EVENT_BUTTON) != 0U) {
            paused ^= 1U;
            led_state = 0U;
            Xil_Out32(LED_DATA, led_state);
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

    TimerHandle_t button_timer = xTimerCreate(
        "Button", pdMS_TO_TICKS(BUTTON_SAMPLE_MS), pdTRUE, NULL, button_callback);

    if (blink_timer == NULL || button_timer == NULL) {
        return 1;
    }

    if (xTaskCreate(led_task, "LED", STACK_WORDS, NULL,
                    tskIDLE_PRIORITY + 1U, &led_task_handle) != pdPASS ||
        xTaskCreate(switch_task, "Switch", STACK_WORDS, NULL,
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

Software Timer ตัวแรกส่งบิต `EVENT_BLINK` เมื่อครบครึ่งคาบ ตัวที่สองอ่านปุ่มและส่ง `EVENT_BUTTON` เฉพาะเมื่อยืนยันการเปลี่ยนจากปล่อยเป็นกด การปล่อยปุ่มต้องผ่านการกรองเช่นกันก่อนรับการกดครั้งต่อไป LED Task เป็นผู้เขียน LED0 ทั้งตอนกะพริบและตอนพัก เมื่อทั้งสองเหตุการณ์มาพร้อมกัน ระบบจัดการปุ่มก่อนและเริ่มกะพริบอีกครั้งในจังหวะ timer ถัดไป

## 3. รันและตรวจผล

โหลด bitstream และรัน application ของ LAB 04 ผ่าน USB-JTAG หากใช้ debugger ให้กด Resume หลังหยุดที่ `main()`

| การทดลอง | ผลที่คาดหวัง |
|---|---|
| เริ่มโดยปล่อย BTNU และ SW0 = 0 | LED0 กะพริบประมาณ 1 Hz |
| กด BTNU ค้าง 1 วินาที | LED0 ดับและอยู่ในโหมดพัก การกดค้างไม่สลับโหมดซ้ำ |
| ปล่อยปุ่ม แล้วกด BTNU อีกครั้ง | LED0 กลับมากะพริบในจังหวะ timer ถัดไป |
| พัก LED0 แล้วเปลี่ยน SW0 เป็น 1 จากนั้นกด BTNU | LED0 กลับมากะพริบประมาณ 4 Hz |

หาก LED กะพริบได้แต่ BTNU ไม่ตอบสนอง ให้ตรวจ M18, การต่อ Concat In1 และความกว้าง 2 บิตของ GPIO ช่อง 1

**แบบฝึกหัด** เพิ่มจำนวนตัวอย่างยืนยันจาก 3 เป็น 5 ที่ช่วงสุ่มเดิม 10 ms แล้วเปรียบเทียบการตอบสนองเมื่อแตะปุ่มสั้น ๆ เวลายืนยันจะอยู่ประมาณ 40–50 ms ขึ้นกับจังหวะที่กดเทียบกับรอบสุ่มอ่าน

## เอกสารอ้างอิง

- [Digilent Nexys A7 Reference Manual](https://digilent.com/reference/_media/reference/programmable-logic/nexys-a7/nexys-a7_rm.pdf)
- [Digilent Nexys A7-100T Master XDC](https://github.com/Digilent/digilent-xdc/blob/master/Nexys-A7-100T-Master.xdc)
- [FreeRTOS Reference Manual — Timers and Task Notifications](https://www.freertos.org/media/2018/FreeRTOS_Reference_Manual_V10.0.0.pdf)
