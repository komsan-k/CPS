# LAB 01 — MicroBlaze บน Nexys A7

ใช้โปรแกรมภาษา C บน MicroBlaze เพื่ออ่านสวิตช์ SW0 และควบคุม LED0 ผ่าน AXI GPIO

**อุปกรณ์และซอฟต์แวร์** Nexys A7-100T, สาย USB สำหรับ JTAG และ Vivado กับ Vitis Unified IDE รุ่นเดียวกัน คู่มือนี้ใช้ **Classic MicroBlaze** และโดเมน **Standalone** โปรแกรมและข้อมูลอยู่ใน Block RAM บน FPGA

## 1. สร้างระบบฮาร์ดแวร์ใน Vivado

1. สร้าง RTL Project สำหรับ **Nexys A7-100T** หากไม่มี board files ให้เลือก part `xc7a100tcsg324-1` โดยตรง จากนั้นเลือก **Create Block Design** แล้วเพิ่ม **MicroBlaze** จาก IP Catalog
2. เลือก **Run Block Automation** ใช้ **Local Memory 32 KB**, เปิด **Peripheral AXI Port** และเก็บ **Debug Module (MDM)** สำหรับดาวน์โหลดโปรแกรมผ่าน JTAG ใช้ clock ภายนอก 100 MHz หากมีตัวเลือกให้แปลงเป็น MicroBlaze V ให้เลือก **Keep Classic MicroBlaze**
3. ตรวจเส้น clock และ reset ใน Block Design ใช้พอร์ต `clk_100mhz` เป็น clock หลักของ MicroBlaze และ AXI GPIO ให้ `slowest_sync_clk` ของ **Processor System Reset** รับ clock เดียวกัน ต่อ `reset_btn` เข้ากับ `ext_reset_in` และตั้ง **External Reset Polarity = Active High** เพราะใช้ BTNC หากใช้ clock จากขา E3 โดยตรง ให้ต่อ `dcm_locked` กับค่าคงที่ 1 หากวงจรมี Clocking Wizard ให้ใช้สัญญาณ `locked` ของบล็อกนั้น ตรวจว่า MicroBlaze และ AXI GPIO รับ reset จากเอาต์พุตที่ถูกขั้วของ Processor System Reset
4. เพิ่ม **AXI GPIO** หนึ่งบล็อก เปิด **Enable Dual Channel** แล้วตั้งค่าตามตาราง ใช้ **Run Connection Automation** ที่ `S_AXI` เพื่อเชื่อมกับ AXI master ของ MicroBlaze ตรวจว่า `s_axi_aclk` และ `s_axi_aresetn` เชื่อมแล้ว

| ช่อง AXI GPIO | การตั้งค่า | พอร์ตภายนอก |
|---|---|---|
| Channel 1 | Width = 1, All Inputs | `sw0[0]` |
| Channel 2 | Width = 1, All Outputs, Default Output Value = 0 | `led0[0]` |

5. ขยายอินเทอร์เฟซ `GPIO` และ `GPIO2` แล้วใช้ **Make External** ที่ขา `gpio_io_i` และ `gpio2_io_o` ตามลำดับ เปลี่ยนชื่อ external ports เป็น `sw0` และ `led0` ตรวจชื่อพอร์ตจริงหลังสร้าง wrapper หากเครื่องมือสร้าง GPIO เป็นพอร์ต scalar ให้ใช้ `{sw0}` และ `{led0}` ใน XDC
6. เปิด **Address Editor** แล้วกำหนด Base Address ของ AXI GPIO เป็น `0x40000000` ใน data address space ของ MicroBlaze หากเลือกใช้ address อื่น ให้แก้ `GPIO_BASE` ในโปรแกรม C ให้ตรงกัน

## 2. เพิ่ม constraints และส่งออก XSA

เพิ่มไฟล์ `nexys_a7_lab01.xdc` ใน **Constraints** ตรวจว่าชื่อพอร์ตใน XDC ตรงกับ HDL wrapper และใช้ constraint สำหรับ clock เดียวต่อพอร์ต

```tcl
set_property -dict {PACKAGE_PIN E3 IOSTANDARD LVCMOS33} [get_ports {clk_100mhz}]
create_clock -name sys_clk -period 10.000 [get_ports {clk_100mhz}]

set_property -dict {PACKAGE_PIN N17 IOSTANDARD LVCMOS33} [get_ports {reset_btn}]
set_property -dict {PACKAGE_PIN J15 IOSTANDARD LVCMOS33} [get_ports {sw0[0]}]
set_property -dict {PACKAGE_PIN H17 IOSTANDARD LVCMOS33} [get_ports {led0[0]}]
```

เลือก **Validate Design** แล้วเลือก **Create HDL Wrapper** ให้ Vivado จัดการ wrapper และตั้งเป็น top-level จากนั้นเลือก **Generate Bitstream** เมื่อเสร็จแล้วไปที่ **File → Export → Export Hardware** และเลือก **Include bitstream** เพื่อสร้างไฟล์ `.xsa`

## 3. เขียนโปรแกรม C ใน Vitis

1. สร้าง **Platform Component** จากไฟล์ `.xsa` เลือก processor `microblaze_0` และระบบปฏิบัติการ **standalone** แล้ว build platform
2. สร้าง **Application Component** แบบ **Empty Application** บน platform และโดเมนดังกล่าว แล้วเพิ่มไฟล์ `main.c` ในโฟลเดอร์ `src`
3. ใส่โค้ดด้านล่าง จากนั้น build ตัว application ต่อสาย USB-JTAG เปิดบอร์ด แล้วเลือก **Run** หรือ **Debug** แบบ hardware หากเริ่มผ่าน debugger ให้กด **Resume** เมื่อหยุดที่ `main()`

```c
#include "xil_io.h"

#define GPIO_BASE       0x40000000U
#define SW_DATA_OFFSET  0x00U
#define LED_DATA_OFFSET 0x08U
#define GPIO_BIT_MASK   0x01U

int main(void)
{
    for (;;) {
        u32 switch_value = Xil_In32(GPIO_BASE + SW_DATA_OFFSET);
        Xil_Out32(GPIO_BASE + LED_DATA_OFFSET, switch_value & GPIO_BIT_MASK);
    }
}
```

โปรแกรมใช้ `Xil_In32` และ `Xil_Out32` เพื่อเข้าถึงรีจิสเตอร์แบบ memory-mapped โดยตรง ทิศทางสัญญาณกำหนดไว้ใน AXI GPIO แล้ว ตำแหน่ง `0x00` คือรีจิสเตอร์ข้อมูลของ Channel 1 และ `0x08` คือรีจิสเตอร์ข้อมูลของ Channel 2

## 4. ทดสอบและบันทึกผล

| การทดสอบ | ผลที่คาดหวัง |
|---|---|
| ตั้ง SW0 = 0 | LED0 ดับ |
| ตั้ง SW0 = 1 | LED0 ติด |
| สลับ SW0 จาก 1 กลับเป็น 0 | LED0 ดับอีกครั้ง |

ถ้า LED0 ค้าง ให้ตรวจว่าโปรแกรมกำลังรันและ FPGA ถูกโปรแกรมด้วย bitstream จาก XSA ของ LAB นี้ จากนั้นเทียบ `GPIO_BASE` กับ Address Editor และตรวจลำดับช่อง GPIO กับชื่อพอร์ตใน XDC

**แบบฝึกหัด** เปลี่ยนบรรทัดเขียนค่าให้ใช้ `(switch_value & GPIO_BIT_MASK) ^ GPIO_BIT_MASK` แล้วบันทึกผลทั้งสองตำแหน่งของ SW0 ค่าที่คาดหวังคือ SW0 = 0 ทำให้ LED0 ติด และ SW0 = 1 ทำให้ LED0 ดับ

## เอกสารอ้างอิง

- [Digilent Nexys A7-100T Master XDC](https://github.com/Digilent/Nexys-A7-100T-GPIO/blob/master/src/constraints/Nexys-A7-100T-Master.xdc)
- [AMD MicroBlaze Block Automation](https://docs.amd.com/r/en-US/ug911-vivado-migration/MicroBlaze-Block-Automation)
- [AMD AXI GPIO Register Space (PG144)](https://docs.amd.com/r/en-US/pg144-axi-gpio/Register-Space)
- [AMD Vitis Unified IDE — Creating a Platform Component from XSA](https://docs.amd.com/r/en-US/ug1400-vitis-embedded/Creating-a-Platform-Component-from-XSA)
