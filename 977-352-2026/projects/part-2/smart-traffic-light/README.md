# 🚦 ESP32 Smart Traffic Light with Pedestrian Button (Two-Lane System)

## 📘 Overview
This project implements a simple two-lane traffic light system using an ESP32, where pedestrians can request to cross the road by pressing a button.

### Features
- Safe traffic light transitions  
- Controlled pedestrian crossing  
- No immediate unsafe switching  

---

## 🛣️ Intersection Design
- **Major Road:** East ↔ West  
- **Minor Road:** North ↔ South  
- **Pedestrian Crossing:** Across the major road  

---

## 🔘 System Concept
When a pedestrian presses the button:
- Request is stored  
- System waits for a safe moment  
- Traffic lights transition properly  
- Pedestrian gets Walk signal  
- System returns to normal  

---

## 🔁 Traffic Light States

### 🟢 State 1: Major Green
- East-West = Green  
- North-South = Red  
- Pedestrian = Don't Walk  

### 🟡 State 2: Major Yellow
- East-West = Yellow  
- North-South = Red  

### 🚶 State 3: Pedestrian Walk
- All traffic = Red  
- Pedestrian = Walk  

### 🟢 State 4: Minor Green
- East-West = Red  
- North-South = Green  

---

## ⏱️ Timing Example

| State | Duration |
|------|--------|
| Major Green | 10–20 s |
| Major Yellow | 3 s |
| Pedestrian Walk | 5–10 s |
| Minor Green | 5–10 s |

---

## 🧠 Control Logic

### Pedestrian Request
```
pedestrian_request = true;
```

### FSM Transition
```
MAJOR GREEN → YELLOW → ALL RED → WALK → ALL RED → MINOR GREEN → REPEAT
```

---

## 💻 Pseudocode
```
if (button_pressed) {
    pedestrian_request = true;
}

switch (state) {
case MAJOR_GREEN:
    if (time_elapsed > MIN_GREEN && pedestrian_request)
        state = MAJOR_YELLOW;
    break;

case MAJOR_YELLOW:
    if (time_elapsed > 3s)
        state = ALL_RED_1;
    break;

case ALL_RED_1:
    if (time_elapsed > 1s)
        state = PEDESTRIAN_WALK;
    break;

case PEDESTRIAN_WALK:
    if (time_elapsed > WALK_TIME) {
        pedestrian_request = false;
        state = ALL_RED_2;
    }
    break;

case ALL_RED_2:
    if (time_elapsed > 1s)
        state = MINOR_GREEN;
    break;

case MINOR_GREEN:
    if (time_elapsed > MINOR_TIME)
        state = MAJOR_GREEN;
    break;
}
```

---

## 🔌 Hardware Setup

### Inputs
- Button → GPIO 14  

### Outputs
**Major Road**
- Green → GPIO 23  
- Yellow → GPIO 19  
- Red → GPIO 18  

**Minor Road**
- Green → GPIO 5  
- Yellow → GPIO 17  
- Red → GPIO 16  

**Pedestrian**
- Walk → GPIO 4  
- Don't Walk → GPIO 2  

---

## ⚠️ Safety Rules
- Enforce minimum green time  
- Always include yellow phase  
- Add all-red delay  
- Do NOT switch immediately  
- Ignore repeated presses  

---

## 🧪 Example Scenario
1. System starts in Major Green  
2. Pedestrian presses button  
3. Request stored  
4. System transitions safely  
5. Pedestrian crosses  
6. System resumes  

---

## 🚀 Extensions
- Dual buttons  
- Buzzer  
- Countdown timer  
- LDR night mode  
- Node-RED integration  
- TinyML control  

---

## 📌 Summary
This system demonstrates:
- Safe pedestrian-aware control  
- FSM-based traffic logic  
- Expandable CPS / Smart City design  
