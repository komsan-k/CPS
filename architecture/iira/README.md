# Mapping the CPS 5C Architecture to IIRA (Industrial Internet Reference Architecture)

This section explains how the **CPS 5C Architecture** maps to the **Industrial Internet Reference Architecture (IIRA)** defined by the Industrial Internet Consortium (IIC).

The goal is to show that the CPS 5C model is **fully compatible with IIRA** and can be used as a **functional design abstraction** for Industrial Internet and Industrial CPS systems.

---

## 1. Overview of IIRA

The **Industrial Internet Reference Architecture (IIRA)** provides a high-level architectural framework for Industrial Internet systems.  
IIRA organizes systems into four main **viewpoints**:

1. **Business Viewpoint** – business objectives, value, policies  
2. **Usage Viewpoint** – system usage scenarios and interactions  
3. **Functional Viewpoint** – functional components and data flow  
4. **Implementation Viewpoint** – technologies and deployment  

Among these, the **Functional Viewpoint** is the most relevant for mapping with the CPS 5C Architecture.

---

## 2. CPS 5C Architecture and IIRA: Conceptual Relationship

- **IIRA** focuses on *what functional domains exist* in an industrial system  
- **5C Architecture** focuses on *how data evolves into intelligence and control*  

Thus, the 5C architecture acts as a **functional intelligence pipeline** inside the broader IIRA framework.

---

## 3. Mapping CPS 5C to the IIRA Functional Viewpoint

The IIRA Functional Viewpoint consists of five key domains:
- Control
- Operations
- Information
- Application
- Business

### CPS 5C ↔ IIRA Functional Domain Mapping

| CPS 5C Level | IIRA Functional Domain | Explanation |
|-------------|------------------------|-------------|
| **Connection** | Control Domain | Interfaces with sensors and actuators; real-time interaction with physical assets |
| **Conversion** | Control / Information Domain | Signal conditioning, preprocessing, and data transformation |
| **Cyber** | Information Domain | Data aggregation, storage, Digital Twin models, system-level context |
| **Cognition** | Application / Business Domain | Analytics, AI, decision support, optimization |
| **Configuration** | Control Domain | Feedback control, actuation, and system reconfiguration |

---

## 4. Closed-Loop Intelligence in IIRA Context

IIRA emphasizes **closed-loop control and optimization** across industrial systems.  
The CPS 5C architecture naturally implements this by:

- Acquiring data at the **Connection** level  
- Refining information at the **Conversion** level  
- Creating system-wide knowledge at the **Cyber** level  
- Making intelligent decisions at the **Cognition** level  
- Executing adaptive actions at the **Configuration** level  

This closed-loop aligns directly with IIRA’s control–information–application interaction model.

---

## 5. Comparison: 5C vs IIRA Roles

| Aspect | CPS 5C Architecture | IIRA |
|------|---------------------|------|
| Nature | Functional intelligence flow | Reference architecture |
| Focus | Data → decision → action | Industrial system viewpoints |
| Granularity | Implementation-oriented | Conceptual / architectural |
| Educational use | Very high | Moderate |
| Industry alignment | Strong | Strong |

---

## 6. Discussion: Why 5C Complements IIRA

The CPS 5C Architecture should be viewed as a **complementary functional abstraction**, not a replacement for IIRA.

- IIRA defines **where functions belong**
- 5C explains **how intelligence and control emerge**

Together, they provide:
- Standards-aligned CPS design
- Clear implementation guidance
- Strong support for Industry 4.0 / Industrial Internet systems

---

## 7. Summary

- The **CPS 5C Architecture maps naturally to IIRA**
- It aligns strongly with the **IIRA Functional Viewpoint**
- It provides an **implementation-ready intelligence pipeline**
- It is suitable for **education, labs, and industrial CPS design**

 ✅ IIRA-aligned | ✅ Industry-ready | ✅ CPS-focused
