# 🎓 Student Mini-Projects Based on Real-World ITS Cities

This section defines **hands-on student mini-projects** inspired by **real-world Intelligent Transportation Systems (ITS) deployments**.  
Each project is scoped for **2–3 weeks**, suitable for **undergraduate or graduate CPS / ITS / Smart City courses**.

---

## 📌 Project Structure (Common to All)
Each mini-project should include:
- Clear **problem statement**
- **CPS 5C mapping**
- Working prototype or simulation
- Evaluation with metrics
- Short discussion on **Transportation 5.0 principles**

---

## 1️⃣ Adaptive Traffic Signals — London, UK

**Problem:**  
Reduce intersection delay using demand-responsive signal control.

**Build:**  
ESP32 (or simulated sensors) → MQTT → Node-RED controller → signal timing logic

**CPS 5C Mapping**
- Connection: Lane sensors
- Conversion: Traffic flow estimation
- Cyber: Intersection state model
- Cognition: Rule-based optimization
- Configuration: Signal phase control

**Tools:** ESP32 / Wokwi, Node-RED, MQTT  
**Deliverables:** Node-RED flow, delay comparison plot, dashboard screenshot

---

## 2️⃣ Congestion Pricing Simulator — Stockholm, Sweden

**Problem:**  
Study traffic reduction under time-varying congestion tolls.

**Build:**  
Zone-based traffic simulator with adjustable pricing policies.

**CPS 5C Mapping**
- Conversion: Vehicle count analysis
- Cyber: Pricing zone model
- Cognition: Policy scoring
- Configuration: Dynamic toll setting

**Tools:** Python, Node-RED Dashboard  
**Deliverables:** Policy comparison report, acceptance discussion

---

## 3️⃣ City Traffic Prediction (AI) — Singapore

**Problem:**  
Predict congestion 10–15 minutes ahead.

**Build:**  
Train a lightweight prediction model and visualize forecasts.

**CPS 5C Mapping**
- Connection: Historical traffic data
- Conversion: Feature extraction
- Cyber: Predictive model
- Cognition: Forecast interpretation

**Tools:** Python (scikit-learn), Node-RED  
**Deliverables:** MAE/RMSE metrics, prediction plots

---

## 4️⃣ Smart Bus Tracking & ETA — Seoul, South Korea

**Problem:**  
Improve public transport experience with accurate ETAs.

**Build:**  
Simulated GPS → ETA computation → passenger dashboard.

**CPS 5C Mapping**
- Connection: GPS data
- Conversion: Speed & delay estimation
- Cyber: Route model
- Cognition: ETA prediction
- Configuration: Passenger alerts

**Tools:** Node-RED, MQTT  
**Deliverables:** ETA accuracy analysis, dashboard

---

## 5️⃣ Autonomous Shuttle Control (V2X) — Phoenix, USA

**Problem:**  
Coordinate autonomous shuttles with traffic signals via V2I.

**Build:**  
Priority request system with safety fallback logic.

**CPS 5C Mapping**
- Connection: V2I messages
- Conversion: Priority detection
- Cyber: Intersection logic
- Cognition: Decision rules
- Configuration: Signal override

**Tools:** Node-RED, MQTT  
**Deliverables:** Latency analysis, safety discussion

---

## 6️⃣ Mobility-as-a-Service Trip Planner — Helsinki, Finland

**Problem:**  
Optimize trips across bus, bike, and walking.

**Build:**  
Multi-criteria routing considering time, cost, and emissions.

**CPS 5C Mapping**
- Cyber: Multimodal network model
- Cognition: Multi-objective optimization
- Configuration: Route recommendation

**Tools:** Python, Node-RED UI  
**Deliverables:** Route comparison table, user-centric analysis

---

## 7️⃣ Green Fleet Optimization — Shenzhen, China

**Problem:**  
Minimize emissions in an all-electric bus fleet.

**Build:**  
Charging schedule optimization and eco-routing.

**CPS 5C Mapping**
- Conversion: Energy consumption analysis
- Cyber: Fleet Digital Twin
- Cognition: Optimization logic
- Configuration: Dispatch & charging plan

**Tools:** Python, Node-RED  
**Deliverables:** Emissions reduction report, charging schedule

---

## 8️⃣ City Mobility Digital Twin — Paris, France

**Problem:**  
Evaluate pedestrian-first mobility policies safely.

**Build:**  
Policy scenario simulation using a city-scale Digital Twin.

**CPS 5C Mapping**
- Cyber: Digital Twin model
- Cognition: Policy evaluation
- Configuration: Recommended actions

**Tools:** Python simulation, Node-RED Dashboard  
**Deliverables:** KPI dashboard, policy impact report

---

## 🧪 Suggested Assessment Rubric
- Technical correctness & CPS loop: **40%**
- Transportation 5.0 alignment: **30%**
- Evaluation & metrics: **20%**
- Presentation & clarity: **10%**

---

## 🎓 Learning Outcomes
After completing any project, students will be able to:
- Apply **CPS 5C architecture** to transportation systems
- Understand real-world **ITS deployments**
- Design **human-centered, sustainable mobility solutions**
- Evaluate trade-offs using data and metrics


