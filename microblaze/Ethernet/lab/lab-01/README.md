# Ethernet LAB 01 — TCP Echo บน Nexys A7-100T

ให้ **MicroBlaze บน Nexys A7-100T** รับข้อความจากคอมพิวเตอร์ผ่านพอร์ต Ethernet แล้วส่งข้อความเดิมกลับ ด้วยตัวอย่าง **lwIP Echo Server** ของ Xilinx เมื่อจบ LAB จะตรวจแยกได้ว่า Ethernet link ขึ้น, IPv4 ติดต่อได้ และ TCP รับส่งข้อมูลได้จริง

**อุปกรณ์** Nexys A7-100T, สาย USB-JTAG/UART, สาย Ethernet, คอมพิวเตอร์ที่มี Ethernet หรือ USB-to-Ethernet และสวิตช์ Ethernet ที่รองรับ 100 Mb/s หากใช้สวิตช์ให้ต่อทั้งสองอุปกรณ์กับสวิตช์ LAN เดียวกัน

**ซอฟต์แวร์สำหรับเส้นทางในคู่มือนี้** Vivado **2019.1** และ Xilinx **SDK 2019.1** พร้อม board files **Nexys A7-100T D.0/1.0** ที่มี eth_rmii และ eth_mdio เส้นทางนี้อิงคู่มือ Nexys 4 DDR ของ Digilent และรายงานการใช้งานกับ Nexys A7-100T หากใช้ Vivado รุ่นใหม่ ให้ตรวจความพร้อมของ IP MII-to-RMII ก่อนเริ่ม เพราะขั้นตอนนี้อาศัย IP และ board interface รุ่นเดิม

## 1. กำหนด IP สำหรับ LAN ทดลอง

ใช้เครือข่ายทดลองแยกหรือเชื่อมอะแดปเตอร์ Ethernet ของคอมพิวเตอร์เข้าบอร์ดโดยตรงหาก link ขึ้น

| อุปกรณ์ | IPv4 | Subnet mask |
|---|---|---|
| Nexys A7 | 192.168.130.10 | 255.255.255.0 |
| คอมพิวเตอร์ | 192.168.130.20 | 255.255.255.0 |

ตั้งค่า IPv4 ของอะแดปเตอร์ Ethernet ที่ต่อกับบอร์ด หากใช้เราเตอร์ในห้องเรียนให้เลือก subnet และที่อยู่ที่ยังว่างสำหรับทั้งสองอุปกรณ์ สำหรับการทดสอบใน subnet เดียวกันปล่อยช่อง gateway ของเครื่องทดสอบว่างได้

## 2. สร้างฮาร์ดแวร์ใน Vivado

ใช้ [คู่มือ MicroBlaze Servers ของ Digilent](https://digilent.com/reference/learn/programmable-logic/tutorials/nexys-4-ddr-getting-started-with-microblaze-servers/start) สำหรับขั้นตอน Block Automation และ DDR2 เมื่อสร้างโปรเจกต์แล้วให้เลือก **Nexys A7-100T board file 1.0** ตรวจว่าหน้า Board แสดง Ethernet RMII และ MDIO หากใช้รุ่นของ board files ที่ไม่มีสองอินเทอร์เฟซนี้ ให้เลือกชุด 1.0 ก่อนสร้าง Block Design

| บล็อก | การเชื่อมต่อหลัก |
|---|---|
| Classic MicroBlaze และ MDM | ใช้ Block Automation เปิด AXI peripheral port, local BRAM สำหรับระบบและ JTAG debug |
| DDR2 MIG | ใช้ board preset ของ Nexys A7-100T เชื่อม AXI memory-mapped ของ MicroBlaze เข้ากับ MIG ให้ SDK วางโปรแกรมและบัฟเฟอร์ lwIP ใน DDR2 |
| AXI Ethernet Lite | ต่อ S_AXI กับ AXI interconnect เปิด **MII Management Module** เพื่อใช้ MDIO |
| MII to RMII | ตั้ง Mode Selection เป็น **Tri-Mode Ethernet MAC** แล้วต่อ MII master ของ Ethernet Lite เข้ากับฝั่ง MII ของตัวแปลง ต่อ RMII ฝั่ง PHY ไปยัง eth_rmii ของบอร์ด |
| MDIO และ reset | ต่อ MDIO ของ Ethernet Lite เข้ากับ eth_mdio ของบอร์ด ต่อ active-low PHY reset ไปทั้ง PHY บนบอร์ดและ rst_n ของตัวแปลง |
| AXI Timer และ AXI INTC | นำ IRQ ของ AXI Timer และ AXI Ethernet Lite เข้า INTC คนละ input ผ่าน Concat แล้วต่อ INTC ไปที่ MicroBlaze INTERRUPT |
| AXI UART Lite | ต่อพอร์ต USB UART สำหรับข้อความเริ่มต้น ตั้ง 115200 baud, 8-N-1 |

**นาฬิกา RMII** ใช้ Clocking Wizard จาก system clock 100 MHz สร้างเอาต์พุต 50 MHz สองชุด ชุดแรกเป็น ref_clk ของ MII-to-RMII ตั้ง phase 0° และอีกชุดเป็น ETH_REFCLK ไปที่ขา CLKIN ของ PHY บนบอร์ด ตั้ง phase +45° ตามคู่มือ Nexys A7 คง reset ไว้จน Clocking Wizard lock ใช้ sys_clk_i, clk_ref_i และ ui_clk ของ DDR2 ตาม preset และการต่อในคู่มือ MIG สำหรับบอร์ด

**ความเร็ว** ใน LAB นี้ตั้งตัวแปลงเป็น **Fixed Speed / Speed 100** และตรวจว่าลิงก์เจรจาได้ 100 Mb/s PHY LAN8720A ของบอร์ดมี PHY address เริ่มต้น 1 แยกความถี่ 50 MHz ของ RMII ออกจาก clock ของบัส AXI

หากตรวจการผูกขาด้วย Master XDC ให้เทียบ RMII กับตารางนี้ ชื่อพอร์ตระดับบนขึ้นกับ HDL wrapper และ board automation

| สัญญาณ PHY | ขา FPGA |
|---|---|
| RXD[0], RXD[1] | C11, D10 |
| TXD[0], TXD[1] | A10, A8 |
| CRS_DV, RX_ER, TX_EN | D9, C10, B9 |
| MDC, MDIO | C9, A9 |
| RESET_N, REFCLK | B3, D5 |

เลือก **Validate Design → Create HDL Wrapper → Generate Bitstream** แล้ว **File → Export → Export Hardware** พร้อม Include bitstream จากนั้นเปิด **Xilinx SDK**

## 3. สร้าง lwIP Echo Server ใน SDK

1. สร้าง **Application Project** บน Classic MicroBlaze เลือก OS **standalone** และ template **lwIP Echo Server** ใช้ template และ BSP ที่มากับ SDK รุ่นเดียวกับที่สร้าง hardware
2. ใน BSP ตั้ง stdout ไปที่ AXI UART Lite และใช้ lwIP IPv4 แบบ RAW API สำหรับตัวอย่างนี้ ปิด DHCP ในการตั้งค่า lwIP เพื่อใช้ static IPv4
3. ใน main.c ของ template ค้นหาค่าตั้งต้น 192.168.1.10 หรือส่วนกำหนด IPv4 แล้วแก้เป็น **192.168.130.10**, netmask **255.255.255.0**, gateway **0.0.0.0** สำหรับ LAN ทดลอง ตรวจว่า linker script วางส่วนหลักของโปรแกรม รวมถึง heap และ stack ใน DDR2
4. Build แล้วเลือก **Run As → Launch on Hardware** ผ่าน JTAG เปิด Serial Terminal ที่ 115200 baud, 8-N-1 ก่อนเริ่มโปรแกรม ตรวจ IP และ TCP port จากข้อความเริ่มต้น ตัวอย่างมาตรฐานใช้ **TCP port 7** หาก template ของชุดเครื่องมือระบุพอร์ตอื่น ให้ใช้พอร์ตที่โปรแกรมรายงาน

## 4. ทดสอบจากคอมพิวเตอร์

เริ่มจากดูไฟ link ที่บอร์ดหรือสวิตช์ แล้วรัน ping 192.168.130.10 จากคอมพิวเตอร์ หาก ping ผ่าน ให้ทดสอบ TCP ด้วย Python 3 ตัวอย่างต่อไปนี้ บันทึกเป็น tcp_echo_check.py แล้วรัน python tcp_echo_check.py

```python
import socket
import sys


def check_echo(host, port):
    message = b"Hello Nexys A7\n"

    with socket.create_connection((host, port), timeout=3) as connection:
        connection.sendall(message)
        reply = bytearray()

        while len(reply) < len(message):
            chunk = connection.recv(len(message) - len(reply))
            if not chunk:
                break
            reply.extend(chunk)

    if reply != message:
        raise SystemExit(f"FAIL: expected {message!r}, received {bytes(reply)!r}")

    print(f"PASS: {host}:{port} returned {bytes(reply)!r}")


if __name__ == "__main__":
    host = sys.argv[1] if len(sys.argv) > 1 else "192.168.130.10"
    port = int(sys.argv[2]) if len(sys.argv) > 2 else 7
    check_echo(host, port)
```

ผลที่คาดหวังคือ PASS และข้อความเดิมครบทุกไบต์ สคริปต์วนรับข้อมูลเพราะ TCP สามารถส่งข้อความกลับมาเป็นหลายช่วงได้

| อาการ | จุดที่ควรตรวจ |
|---|---|
| ไฟ link ไม่ขึ้น | สาย, พอร์ตสวิตช์, ETH_RSTN, REFCLK 50 MHz และการตั้งความเร็ว |
| ไฟ link ขึ้น แต่ Serial ไม่แสดง IP | การโหลดโปรแกรมผ่าน JTAG, stdout ของ BSP และการเริ่มทำงานของ DDR2 MIG |
| Serial แสดง IP แต่ ping ไม่ผ่าน | IP/subnet ของอะแดปเตอร์ Ethernet ที่ใช้อยู่, MDIO, IRQ ของ MAC และการผูกขา RMII |
| Ping ผ่าน แต่ TCP ไม่ echo | พอร์ตที่ echo server รายงาน, การเริ่มโปรแกรมบนบอร์ด และผล FAIL/timeout ของสคริปต์ |

**แบบฝึกหัด** เปลี่ยนข้อความในตัวแปร message แล้วทดสอบซ้ำ บันทึก IP ของบอร์ด พอร์ต TCP และข้อความที่รับกลับ

## เอกสารอ้างอิง

- [Digilent Nexys A7 Reference Manual — Ethernet PHY และ DDR2](https://digilent.com/reference/_media/reference/programmable-logic/nexys-a7/nexys-a7_rm.pdf)
- [Digilent Forum — รายงานการใช้ Nexys A7-100T กับ Vivado 2019.1 และ board file D.0/1.0](https://forum.digilent.com/topic/30436-using-ethernet-on-nexys-a7/)
- [AMD Vivado 2019.1 Downloads](https://www.amd.com/en/support/downloads/adaptive-socs-and-fpgas/development-tools/2019-1.html)
- [AMD MII to RMII Product Guide PG146](https://docs.amd.com/api/khub/documents/PfuCQ6MEY~3Qz2MUV_qp4A/content)
- [Digilent Nexys A7-100T Master XDC](https://github.com/Digilent/digilent-xdc/blob/master/Nexys-A7-100T-Master.xdc)
- [AMD/Xilinx lwIP Echo Server source](https://github.com/Xilinx/embeddedsw/tree/master/lib/sw_apps/lwip_echo_server)

