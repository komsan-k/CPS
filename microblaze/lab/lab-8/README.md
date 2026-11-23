# LAB 08 — ควบคุมการไล่ความสว่าง LED ด้วย Verilog FSM

ต่อจาก [LAB 07 — ไล่ความสว่างด้วย Timer Interrupt และ PWM](https://chatgpt.com/library?file_id=libfile_b2e66ad84b248191a026435466a37f0d&account_id=personal) ใน LAB นี้ **Verilog FSM** เดินลำดับความสว่างทุก 250 ms ส่วน MicroBlaze อ่านคำสั่ง UART และเขียนรีจิสเตอร์ AXI GPIO นักศึกษาจะได้เปรียบเทียบพฤติกรรมของทั้งสอง LAB เมื่อหยุด CPU ใน debugger

**อุปกรณ์** Nexys A7-100T, USB-JTAG, โปรเจกต์ LAB 07, Vivado และ Vitis Unified IDE รุ่นเดียวกัน ใช้ Classic MicroBlaze และโดเมน Standalone

| คำสั่ง | ผลที่คาดหวัง |
|---|---|
| `f` หรือ `F` | เริ่มลำดับ 0%, 25%, 50%, 75%, 100%, 75%, 50%, 25% แล้ววนทุกประมาณ 2 s |
| `0`–`4` | เลือก duty cycle คงที่ตั้งแต่ 0% ถึง 100% ทีละ 25% |
| `s` หรือ `S` | เริ่มใช้ SW0 เลือก 25% หรือ 75% |
| `?` | แสดงรายการคำสั่ง |

## 1. สร้าง RTL สำหรับกำหนดระดับ PWM

วงจรมีสองสถานะคือ **MANUAL** และ **FADE** ในสถานะ FADE ตัวนับเพิ่มดัชนีลำดับทุก 25,000,000 clocks ที่ clock 100 MHz ดัชนี 3 บิตวนกลับเป็นศูนย์อัตโนมัติหลังผ่านแปดขั้น

| บิตของ AXI GPIO ช่อง 2 | หน้าที่ |
|---|---|
| `[2:0]` | ระดับ PWM คงที่ 0–4 |
| `[3]` | 0 เลือกระดับคงที่ และ 1 เลือก FADE |
| `[4]` | ซอฟต์แวร์กลับค่าบิตนี้ทุกครั้งที่รับ `f` เพื่อเริ่ม FADE ใหม่ได้แม้บิตเปิดโหมดยังคงเป็น 1 |

ในสำเนาโปรเจกต์ LAB 07 เพิ่ม `fade_controller.v` ใน **Design Sources** แล้วใช้ **Add Module** เพื่อเพิ่มใน Block Design

```verilog
module fade_controller #(
    parameter [31:0] STEP_CYCLES = 32'd25_000_000
)(
    input  wire       clk,
    input  wire       resetn,
    input  wire [4:0] control_word,
    output wire [2:0] duty_level
);
    localparam MANUAL = 1'b0, FADE = 1'b1;

    reg        state;
    reg        last_restart;
    reg [31:0] step_count;
    reg [2:0]  step_index;

    wire restart_changed = control_word[4] != last_restart;
    wire [2:0] fade_level = (step_index <= 3'd4) ?
                            step_index : (3'd7 - step_index + 3'd1);

    assign duty_level = (state == FADE) ? fade_level : control_word[2:0];

    always @(posedge clk) begin
        if (!resetn) begin
            state        <= MANUAL;
            last_restart <= 1'b0;
            step_count   <= 32'd0;
            step_index   <= 3'd0;
        end else begin
            last_restart <= control_word[4];

            case (state)
                MANUAL: begin
                    if (control_word[3]) begin
                        state      <= FADE;
                        step_count <= 32'd0;
                        step_index <= 3'd0;
                    end
                end
                FADE: begin
                    if (!control_word[3]) begin
                        state <= MANUAL;
                    end else if (restart_changed) begin
                        step_count <= 32'd0;
                        step_index <= 3'd0;
                    end else if (step_count == STEP_CYCLES - 32'd1) begin
                        step_count <= 32'd0;
                        step_index <= step_index + 3'd1;
                    end else begin
                        step_count <= step_count + 32'd1;
                    end
                end
            endcase
        end
    end
endmodule
```

ตั้ง `STEP_CYCLES` เป็นจำนวนเต็มบวก วงจร `led_pwm` จาก LAB 06 ยังคงรับ duty level ใหม่ที่ขอบคาบ PWM 1 ms

## 2. ต่อสัญญาณและสร้าง XSA

1. ตั้ง **AXI GPIO Channel 2** เป็น **All Outputs, Width = 5, Default Output Value = 0** แล้วต่อ `gpio2_io_o[4:0]` ไปที่ `fade_controller/control_word[4:0]`
2. ต่อ `fade_controller/duty_level[2:0]` ไปที่ `led_pwm/duty_level[2:0]` ใช้ clock 100 MHz และ `peripheral_aresetn` เส้นเดียวกันสำหรับ AXI GPIO, FSM และ PWM
3. ใช้ XDC ของ LAB 07 สำหรับ SW0, UART และ LED0 ตัวอย่างใช้ GPIO base `0x40000000` กับ UART base `0x40600000` ตรวจค่าจริงใน Address Editor เก็บ AXI Timer และ AXI INTC ของ LAB 07 ไว้ใน Block Design ได้ เฟิร์มแวร์ LAB 08 ใช้วงจร fade เป็นตัวกำหนดช่วงเวลา
4. เลือก **Validate Design → Generate Bitstream → Export Hardware** พร้อม **Include bitstream**

## 3. เขียนโปรแกรมใน Vitis

สร้าง platform จาก XSA ของ LAB 08 แล้ว build แบบ Standalone สร้าง Empty Application และเพิ่ม `src/main.c` ด้านล่าง

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

#define FADE_ENABLE  (1U << 3)
#define FADE_RESTART (1U << 4)

enum control_mode { MODE_SWITCH, MODE_MANUAL, MODE_FADE };

static enum control_mode mode = MODE_SWITCH;
static u32 manual_level;
static u32 restart_token;

static u32 selected_control(void)
{
    if (mode == MODE_FADE) {
        return FADE_ENABLE | restart_token;
    }

    u32 level = manual_level;
    if (mode == MODE_SWITCH) {
        level = (Xil_In32(SW_DATA) & SW0_MASK) == 0U ? 1U : 3U;
    }
    return restart_token | level;
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
        restart_token ^= FADE_RESTART;
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

    u32 applied_control = selected_control();
    Xil_Out32(PWM_SELECT, applied_control);
    uart_print("LAB 08 ready. Send ? for help.\r\n");

    for (;;) {
        if ((Xil_In32(UART_STATUS) & UART_RX_VALID) != 0U) {
            handle_command((unsigned char)Xil_In32(UART_RX));
        }

        u32 control = selected_control();
        if (control != applied_control) {
            Xil_Out32(PWM_SELECT, control);
            applied_control = control;
        }
    }
}
```

## 4. ทดสอบและเปรียบเทียบกับ LAB 07

ตั้ง Serial Terminal เป็น **115200 baud, 8-N-1, flow control = None** แล้วโหลด bitstream และรัน application ผ่าน JTAG

| การทดสอบ | ผลที่คาดหวัง |
|---|---|
| ส่ง `f` แล้วสังเกตห้ารอบ | ได้ประมาณ 10 s จากขอบที่เริ่มดับครั้งแรกถึงครั้งที่หก |
| ส่ง `f` อีกครั้งระหว่าง LED สว่าง | เริ่มลำดับใหม่จาก 0% |
| ส่ง `2` แล้วดูผลอย่างน้อย 2 s | คงที่ที่ 50% |
| ส่ง `s` แล้วสลับ SW0 | เปลี่ยนระหว่าง 25% และ 75% |
| ส่ง `f` แล้วสั่ง Suspend เฉพาะ MicroBlaze โดยไม่ reset ระบบ | ลำดับความสว่างเดินต่อใน LAB 08 ส่วน LAB 07 ค้างที่ duty cycle ล่าสุดจน CPU กลับมาทำงาน |

**แบบฝึกหัด** ลดเวลาให้เหลือ 1 s ต่อรอบโดยคง PWM ที่ 1 kHz คำนวณค่า `STEP_CYCLES` ใหม่และวัดห้ารอบ ค่าที่คาดหวังคือ 12,500,000 clocks ต่อขั้น และเวลาประมาณ 5 s สำหรับห้ารอบ

## เอกสารอ้างอิง

- [AMD Vivado UG994 — Referencing a Module](https://docs.amd.com/r/en-US/ug994-vivado-ip-subsystems/Referencing-a-Module)
- [AMD AXI GPIO PG144 — Register Space](https://docs.amd.com/r/en-US/pg144-axi-gpio/Register-Space)
