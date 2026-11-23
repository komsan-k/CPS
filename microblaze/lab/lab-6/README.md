# LAB 06 — ปรับความสว่าง LED ด้วย PWM บน Nexys A7

ต่อจาก [LAB 05 — MicroBlaze ควบคุมวงจร Verilog](https://chatgpt.com/library?file_id=libfile_c01a7f5f13988191bca3de25f0f24347&account_id=personal) ใน LAB นี้ MicroBlaze รับคำสั่งจาก Serial Terminal แล้วส่งระดับความสว่างผ่าน AXI GPIO ให้ RTL สร้าง **PWM ความถี่ 1 kHz** ที่ LED0 การเปลี่ยน duty cycle จะมีผลเมื่อเริ่มคาบ PWM ถัดไป

**อุปกรณ์** Nexys A7-100T, สาย USB-JTAG ที่ส่งข้อมูลได้, โปรเจกต์ LAB 05, Vivado และ Vitis Unified IDE รุ่นเดียวกัน และโปรแกรม Serial Terminal

| คำสั่ง | ค่า GPIO ช่อง 2 | Duty cycle | เวลาที่ LED0 เป็น High ต่อคาบ 1 ms |
|---|---:|---:|---:|
| 0 | 0 | 0% | 0 µs |
| 1 | 1 | 25% | 250 µs |
| 2 | 2 | 50% | 500 µs |
| 3 | 3 | 75% | 750 µs |
| 4 | 4 | 100% | 1,000 µs |
| s หรือ S | 1 หรือ 3 | SW0 = 0 ใช้ 25% และ SW0 = 1 ใช้ 75% | 250 หรือ 750 µs |
| ? | — | แสดงรายการคำสั่ง | — |

เริ่มต้นในโหมด SW0 ค่าร้อยละในตารางคือสัดส่วนเวลาที่ขา LED เป็น High การรับรู้ความสว่างด้วยสายตาอาจเปลี่ยนไม่เป็นสัดส่วนเดียวกัน

## 1. เพิ่มวงจร PWM ใน Vivado

ที่ clock 100 MHz คาบละ 1 ms มี 100,000 clocks แบ่งเป็น 4 ช่วง ช่วงละ 25,000 clocks ค่า duty_level กำหนดจำนวนช่วงที่ LED0 เป็น High เช่น ค่า 3 ใช้ช่วงที่ 0, 1 และ 2 รวม 75,000 clocks โมดูลเก็บค่าที่รับจาก AXI GPIO ใน active_level เมื่อจบช่วงที่ 3 เพื่อใช้ตลอดคาบถัดไป

เปิดสำเนาโปรเจกต์ LAB 05 เพิ่ม led_pwm.v ใน **Design Sources**

```verilog
module led_pwm #(
    parameter [31:0] QUARTER_CYCLES = 32'd25_000
)(
    input  wire       clk,
    input  wire       resetn,
    input  wire [2:0] duty_level,
    output reg        led
);
    reg [31:0] quarter_count;
    reg [1:0]  quarter_index;
    reg [2:0]  active_level;

    always @(posedge clk) begin
        if (!resetn) begin
            quarter_count <= 32'd0;
            quarter_index <= 2'd0;
            active_level  <= 3'd0;
            led           <= 1'b0;
        end else begin
            led <= ({1'b0, quarter_index} < active_level);

            if (quarter_count == QUARTER_CYCLES - 32'd1) begin
                quarter_count <= 32'd0;
                quarter_index <= quarter_index + 2'd1;
                if (quarter_index == 2'd3) begin
                    active_level <= (duty_level > 3'd4) ? 3'd4 : duty_level;
                end
            end else begin
                quarter_count <= quarter_count + 32'd1;
            end
        end
    end
endmodule
```

ค่าเริ่มต้นของ LED0 คือดับ ค่าจาก GPIO ที่เกิน 4 ถูกจำกัดไว้ที่ 100% พารามิเตอร์ QUARTER_CYCLES ต้องมีค่าอย่างน้อย 1

## 2. เชื่อมโมดูลใน Block Design

1. ลบ instance hw_blinker จาก Block Design ของสำเนาโปรเจกต์ แล้วเลือก **Add Module → led_pwm** เก็บ external port led0 เดิมไว้เพื่อต่อเข้ากับโมดูลใหม่
2. ตั้ง **AXI GPIO Channel 2** เป็น **All Outputs, Width = 3, Default Output Value = 0** ช่อง 1 ยังรับ SW0 ขนาด 1 บิต การเขียนค่าที่ offset 0x08 ของ GPIO จะส่งค่า 3 บิตไปที่ duty_level
3. ต่อสัญญาณดังตาราง AXI GPIO และ led_pwm ต้องใช้ clock 100 MHz เส้นเดียวกัน

| ต้นทาง | ปลายทาง |
|---|---|
| Clock เดียวกับ AXI GPIO s_axi_aclk | led_pwm/clk |
| Processor System Reset → peripheral_aresetn | led_pwm/resetn |
| AXI GPIO ช่อง 2 → gpio2_io_o[2:0] | led_pwm/duty_level[2:0] |
| led_pwm/led | External port led0 |

4. ใช้ XDC เดิมของ LAB 05 โดยตรวจว่า led0 ยังเป็น scalar ที่ขา H17 ตรวจ base address ของ AXI GPIO และ AXI UART Lite ใน **Address Editor**
5. เลือก **Validate Design → Generate Bitstream → Export Hardware** พร้อม **Include bitstream** เพื่อสร้าง XSA ของ LAB 06

## 3. เขียนโปรแกรม MicroBlaze

สร้าง **Platform Component** จาก XSA ใหม่ ใช้ Classic MicroBlaze และโดเมน Standalone แล้ว build platform สร้าง **Empty Application** และบันทึกโค้ดต่อไปนี้เป็น src/main.c แก้ GPIO_BASE และ UART_BASE หากค่าจริงใน Address Editor ต่างจากตัวอย่าง

```c
#include "xil_io.h"

#define GPIO_BASE         0x40000000U
#define UART_BASE         0x40600000U

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

static u32 use_switch = 1U;
static u32 manual_level;

static u32 selected_level(void)
{
    if (use_switch != 0U) {
        return (Xil_In32(SW_DATA) & SW0_MASK) == 0U ? 1U : 3U;
    }
    return manual_level;
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
        use_switch = 0U;
        uart_print(replies[manual_level]);
    } else if (command == 's' || command == 'S') {
        use_switch = 1U;
        uart_print("OK SW0 mode\r\n");
    } else if (command == '?') {
        uart_print("0: 0% | 1: 25% | 2: 50% | 3: 75% | 4: 100% | s: SW0\r\n");
    } else if (command != '\r' && command != '\n') {
        uart_print("Unknown command. Send ?\r\n");
    }
}

int main(void)
{
    Xil_Out32(UART_CONTROL, UART_RESET_FIFOS);

    u32 applied_level = selected_level();
    Xil_Out32(PWM_SELECT, applied_level);
    uart_print("LAB 06 ready. Send ? for help.\r\n");

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

## 4. ทดสอบและบันทึกผล

โหลด bitstream ของ LAB 06 และรันโปรแกรมผ่าน USB-JTAG เปิด Serial Terminal ที่ **115200 baud, 8-N-1, flow control = None** ส่งคำสั่งเป็นอักขระเดี่ยวได้เลย

1. ส่ง 0 ถึง 4 ทีละตัว ตรวจว่าข้อความตอบตรงกับตาราง และ LED0 เปลี่ยนจากดับไปสว่างเต็มระดับ
2. ส่ง 2 แล้วเปลี่ยน SW0 ทั้งสองตำแหน่ง LED0 ควรอยู่ที่ระดับ 50% ส่ง s แล้วเปลี่ยน SW0 อีกครั้ง LED0 ควรเปลี่ยนระหว่าง 25% และ 75%
3. ถ้ามี ILA ให้ใช้ clock 100 MHz จับสัญญาณ led สำหรับคำสั่ง 1, 2 และ 3 ใช้ capture depth อย่างน้อย 131,072 samples พร้อม trigger ขอบขาขึ้นและตั้งตำแหน่ง trigger ไว้ช่วงต้นของ buffer เพื่อให้เห็นขอบขาขึ้นถัดไป วัดคาบจากขอบขึ้นถึงขอบขึ้น และวัดช่วง High จากขอบขึ้นถึงขอบลง สำหรับ 0% และ 100% ให้ตรวจระดับคงที่แทนการวัดจากขอบสัญญาณ

| คำสั่ง | คาบที่คาดหวัง | ช่วง High ที่คาดหวัง | คาบที่วัดได้ | ช่วง High ที่วัดได้ |
|---|---:|---:|---:|---:|
| 1 | 1,000 µs | 250 µs | | |
| 2 | 1,000 µs | 500 µs | | |
| 3 | 1,000 µs | 750 µs | | |

**แบบฝึกหัด** เพิ่มความถี่ PWM เป็น 2 kHz โดยคงระดับ duty cycle ทั้งห้าระดับ คำนวณ QUARTER_CYCLES ก่อนแก้ RTL ค่าที่คาดหวังคือ 12,500 clocks ต่อช่วงย่อยและคาบเต็ม 500 µs

## เอกสารอ้างอิง

- [AMD Vivado UG994 — Referencing a Module](https://docs.amd.com/r/en-US/ug994-vivado-ip-subsystems/Referencing-a-Module)
- [AMD AXI GPIO PG144 — Register Space](https://docs.amd.com/r/en-US/pg144-axi-gpio/Register-Space)
- [AMD Integrated Logic Analyzer PG172](https://docs.amd.com/api/khub/documents/5NSghh~lEMArDasdZkRVxw/content)
