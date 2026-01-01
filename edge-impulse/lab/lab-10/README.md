# Lab 10 – Energy-Aware and Resource-Constrained CPS (Efficiency & Sustainability)

## 🎯 Objective

The objectives of this laboratory are to:

- Understand **energy and resource constraints** in Cyber-Physical Systems (CPS)
- Analyze **power consumption, memory usage, and latency** on ESP32
- Optimize **Edge Impulse models** for low-power operation
- Apply **TinyML optimization techniques** for sustainable CPS design
- Evaluate trade-offs between accuracy, latency, and energy

---

## 🧠 Background Theory

### 1. Energy Constraints in CPS

Many CPS nodes operate on:
- Batteries
- Energy harvesting
- Strict power budgets

Energy efficiency is critical for:
- Long-term deployment
- Reliability
- Sustainability

---

### 2. Resource Constraints on Edge Devices

Edge devices such as ESP32 have:
- Limited RAM and Flash
- Limited CPU frequency
- Real-time execution constraints

CPS intelligence must be **resource-aware**.

---

### 3. TinyML Optimization Principles

Key optimization techniques include:
- Model size reduction
- INT8 quantization
- Efficient DSP blocks
- Reduced inference frequency
- Duty cycling

Edge Impulse provides built-in tools to analyze these constraints.

---

## 🧱 CPS Focus of This Lab

- **Cross-layer optimization** (C2–C5)
- Sustainable CPS design
- Performance–energy trade-offs

---

## ⚙️ Edge Impulse Focus

- Model performance profiling
- RAM and Flash analysis
- Latency measurement
- Optimization and redeployment

---

## 🧰 Hardware Requirements

- ESP32 development board
- Sensor and actuator from previous labs
- USB cable
- Optional: power measurement tool

---

## 🧪 Software Requirements

- Arduino IDE or PlatformIO
- Edge Impulse inference library
- Edge Impulse Studio

---

## 🔧 Experimental Procedure

### Step 1: Baseline Measurement
1. Deploy current model to ESP32.
2. Measure inference latency.
3. Record RAM and Flash usage.
4. Observe power behavior.

---

### Step 2: Model Optimization
1. Enable quantization.
2. Reduce model complexity.
3. Adjust DSP parameters.
4. Rebuild and redeploy.

---

### Step 3: Compare Performance
1. Compare accuracy before and after optimization.
2. Measure latency improvement.
3. Estimate energy savings.

---

### Step 4: Duty Cycling Strategy
1. Reduce inference frequency.
2. Implement sleep modes.
3. Measure power reduction.

---

## 📊 Results and Observations

Students should observe:
- Reduced memory footprint
- Lower inference latency
- Improved energy efficiency
- Acceptable accuracy trade-offs

---

## 📝 Discussion Questions

1. Why is energy efficiency critical in CPS?
2. How does quantization affect accuracy?
3. What trade-offs are acceptable in safety-critical CPS?
4. How can duty cycling improve CPS lifetime?

---

## 🎓 Learning Outcomes

After completing this lab, students will be able to:
- Analyze CPS resource constraints
- Optimize TinyML models for ESP32
- Evaluate energy–performance trade-offs
- Design sustainable CPS nodes

---

## 🧾 Conclusion

This lab demonstrated how CPS intelligence must be designed with **energy and resource awareness**. Through optimization and careful trade-offs, Edge Impulse enables sustainable CPS deployment on constrained devices.

---

## 📚 References

1. Edge Impulse Documentation  
2. TinyML Optimization Literature  
3. Energy-Efficient CPS Research  

---

### ▶️ Next Lab

**Lab 11 – Adaptive CPS: Model Update and Online Learning**
