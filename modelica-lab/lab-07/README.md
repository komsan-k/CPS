# 🧩 Lab 7: Digital Twin Synchronization and Predictive Maintenance in CPS

### 🎯 Objective
Students will learn to:
1. Synchronize a digital twin with a physical system in real time.
2. Detect degradation and performance drift using twin residuals.
3. Apply predictive maintenance and early fault alert mechanisms.

---

## 1. Prerequisites
- Completion of Lab 6: Cloud-Connected CPS.
- Access [OpenModelica Notebook](https://notebook.openmodelica.org).

---

## 2. Background
Predictive maintenance uses digital twins to estimate system health and forecast failures.

### System Concept
```
[Physical System] → Sensor Data → [Digital Twin]
                              ↓
                       Residuals & Health Index
                              ↓
                     Maintenance Decision Logic
```

---

## 3. Degrading Plant Model
```modelica
model DegradingPlant
  import MyLibrary.*;
  Mass m1(m=1);
  Spring s1(k=2);
  Damper d1(c=0.5);
  ForceActuator act;
  Real k_actual;
equation
  s1.x = m1.x;
  d1.v = m1.v;
  k_actual = s1.k * (1 - 0.05 * time); // stiffness degradation
  m1.F = act.F - k_actual*m1.x - d1.c*m1.v;
  act.u = 1.0;
end DegradingPlant;
```

---

## 4. Ideal Digital Twin Model
```modelica
model IdealTwin
  parameter Real k_nom = 2;
  parameter Real c_nom = 0.5;
  parameter Real m_nom = 1;
  Real x(start=0);
  Real v(start=0);
  input Real F;
equation
  der(x) = v;
  m_nom*der(v) + c_nom*v + k_nom*x = F;
end IdealTwin;
```

---

## 5. Synchronization Between Plant and Twin
```modelica
model SynchronizedTwinSystem
  DegradingPlant plant;
  IdealTwin twin;
  Real residual;
  Real health_index;
equation
  twin.F = plant.act.F;
  residual = abs(plant.m1.x - twin.x);
  health_index = exp(-residual);
end SynchronizedTwinSystem;
```

Run:
```modelica
simulate(SynchronizedTwinSystem, stopTime=10)
plot({plant.m1.x, twin.x})
```

✅ Twin tracks the plant; residual increases with degradation.

---

## 6. Predictive Maintenance Logic
```modelica
model PredictiveMaintenance
  SynchronizedTwinSystem sys;
  parameter Real threshold = 0.8;
  Boolean maintenanceAlert(start=false);
equation
  when sys.health_index < threshold then
    maintenanceAlert = true;
  end when;
end PredictiveMaintenance;
```

Run:
```modelica
simulate(PredictiveMaintenance, stopTime=10)
plot(sys.health_index)
```

---

## 7. Maintenance Data Logger
```modelica
model MaintenanceDataLogger
  PredictiveMaintenance pm;
  parameter Real Ts = 0.5;
  String filename = "maintenance_data.csv";
  Integer fileID;
initial algorithm
  fileID := OpenModelica.Scripting.openFile(filename, "w");
  OpenModelica.Scripting.writeFile(fileID, "time,health_index,residual\n");
equation
  when sample(0, Ts) then
    OpenModelica.Scripting.writeFile(fileID,
      String(time) + "," + String(pm.sys.health_index) + "," +
      String(pm.sys.residual) + "\n");
  end when;
end MaintenanceDataLogger;
```

---

## 8. Health Index Interpretation
| Health Index (h) | Status | Action |
|-------------------|---------|--------|
| h > 0.9 | Healthy | Normal operation |
| 0.7 < h < 0.9 | Slight degradation | Monitor |
| 0.4 < h < 0.7 | Warning | Schedule maintenance |
| h < 0.4 | Fault | Immediate service |

---

## 9. Trend Predictor
```modelica
model TrendPredictor
  PredictiveMaintenance pm;
  Real dh;
  Real t_fail;
equation
  dh = der(pm.sys.health_index);
  t_fail = if dh < 0 then pm.sys.health_index / abs(dh) else 0;
end TrendPredictor;
```

Run:
```modelica
simulate(TrendPredictor, stopTime=10)
plot({pm.sys.health_index, t_fail})
```

✅ t_fail estimates remaining time to failure (RTF).

---

## 10. Example Output Data
| time (s) | health_index | residual |
|-----------|---------------|-----------|
| 0.0 | 1.000 | 0.000 |
| 2.0 | 0.952 | 0.048 |
| 4.0 | 0.831 | 0.185 |
| 6.0 | 0.622 | 0.476 |
| 8.0 | 0.401 | 0.756 |

---

## 11. Exercises
1. Modify degradation rate to 0.02 * time.
2. Add noise to measurement and observe twin robustness.
3. Change health metric to quadratic error: h = exp(-residual^2).
4. Extend twin to estimate both x and v.
5. Connect logger to ThingSpeak for live visualization.

---

## 12. Learning Outcomes
- Synchronize digital and physical systems.
- Calculate residuals and health indices.
- Predict failure using degradation trends.
- Generate maintenance alerts.
- Integrate predictive maintenance with IoT dashboards.

---

## 13. Next Lab
**Lab 8:** Cyber-Physical Fault Detection and Self-Healing Control.
