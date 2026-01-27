# Unified Mapping of CPS 5C Architecture with ISO/IEC 30141, RAMI 4.0, and IIRA

This section provides a **unified comparison** of the **CPS 5C Architecture** with three major international and industrial reference architectures:

- **ISO/IEC 30141 – Internet of Things Reference Architecture**
- **RAMI 4.0 – Reference Architectural Model Industrie 4.0**
- **IIRA – Industrial Internet Reference Architecture (IIC)**

The goal is to show how these frameworks **complement each other**, with CPS 5C focusing on *intelligence flow*, while the others provide *structural and architectural viewpoints*.

---

## 1. Why a Unified Comparison Matters

Modern Cyber-Physical Systems must be:

- Interoperable across vendors and platforms  
- Scalable from labs to industrial deployments  
- Standards-compliant and future-proof  

Each architecture addresses these needs from a different perspective.  
A unified comparison helps designers, students, and practitioners **see how they fit together**.

---

## 2. Mapping the 5C Architecture to ISO/IEC 30141

ISO/IEC 30141 defines a layered IoT reference architecture consisting of **Device**, **Gateway**, **Data Management**, **Application**, and **Business** layers.

### CPS 5C ↔ ISO/IEC 30141 Mapping

| CPS 5C Level | ISO/IEC 30141 Layer | Explanation |
|-------------|---------------------|-------------|
| **Connection** | Device Layer | Sensors and actuators interface with the physical environment |
| **Conversion** | Gateway / Data Processing | Data preprocessing, normalization, protocol translation |
| **Cyber** | Data Management / Application | Aggregated data, Digital Twins, system-level models |
| **Cognition** | Application / Business | Analytics, decision support, optimization |
| **Configuration** | Feedback to Device Layer | Control actions completing the closed-loop |

### Key Insight

This mapping shows how the **5C architecture operationalizes ISO/IEC 30141**, transforming an abstract IoT layering concept into a **concrete end-to-end CPS workflow**.

---

## 3. Mapping the 5C Architecture to RAMI 4.0

RAMI 4.0 is a **three-dimensional reference model** defined along:

- **Hierarchy Levels**
- **Life Cycle & Value Stream**
- **Layers**

The CPS 5C architecture primarily aligns with the **Layers axis** of RAMI 4.0.

### CPS 5C ↔ RAMI 4.0 Mapping

| CPS 5C Level | RAMI 4.0 Layers | Explanation |
|-------------|-----------------|-------------|
| **Connection** | Asset / Integration | Physical assets are sensed and digitally connected |
| **Conversion** | Communication / Information | Data transformation and structured representation |
| **Cyber** | Information / Functional | System integration, Digital Twin functionality |
| **Cognition** | Functional / Business | Intelligent analytics and decision-making |
| **Configuration** | Functional → Asset | Adaptive and autonomous control of physical processes |

### Key Insight

Mapping 5C to RAMI 4.0 ensures alignment with **Industry 4.0 principles**, while preserving the **closed-loop intelligence** that is central to CPS design.

---

## 4. Mapping the 5C Architecture to IIRA

The **Industrial Internet Reference Architecture (IIRA)**, defined by the **Industrial Internet Consortium (IIC)**, describes industrial systems using multiple architectural viewpoints.  
Among these, the **Functional Viewpoint** is the most relevant for CPS mapping.

The IIRA Functional Viewpoint is organized into five functional domains:

- Control  
- Operations  
- Information  
- Application  
- Business  

### CPS 5C ↔ IIRA Functional Viewpoint Mapping

| CPS 5C Level | IIRA Functional Domain | Explanation |
|-------------|------------------------|-------------|
| **Connection** | Control Domain | Real-time interaction with sensors and actuators |
| **Conversion** | Control / Information | Signal conditioning, preprocessing, data transformation |
| **Cyber** | Information Domain | Data aggregation, storage, Digital Twin context |
| **Cognition** | Application / Business | Analytics, optimization, decision support |
| **Configuration** | Control Domain | Feedback control and system reconfiguration |

### Key Insight

While **IIRA defines where industrial functions reside**, the **5C architecture explains how intelligence emerges and flows** across those functions.  
Together, they provide a **complete architectural view for large-scale industrial CPS design**.

## 5. Unified Comparison Table: CPS 5C vs ISO/IEC 30141 vs RAMI 4.0 vs IIRA

| **CPS 5C Architecture** | **Primary Role (5C)** | **ISO/IEC 30141** | **RAMI 4.0** | **IIRA (Functional Viewpoint)** |
|-------------------------|----------------------|-------------------|--------------|---------------------------------|
| **Connection** | Physical sensing & data acquisition | Device Layer | Asset / Integration Layers | Control Domain |
| **Conversion** | Data preprocessing & feature extraction | Gateway / Data Processing | Communication / Information Layers | Control / Information Domains |
| **Cyber** | System-level integration & Digital Twin | Data Management / Application Layers | Information / Functional Layers | Information Domain |
| **Cognition** | Intelligence, analytics & decision-making | Application / Business Layers | Functional / Business Layers | Application / Business Domains |
| **Configuration** | Feedback control & adaptation | Feedback to Device Layer | Functional → Asset Layers | Control Domain |

---

## 6. Key Observations

- **CPS 5C** explains *how data becomes intelligence and action*  
- **ISO/IEC 30141** defines *layered IoT system structure*  
- **RAMI 4.0** defines *where functions live in Industry 4.0 systems*  
- **IIRA** defines *functional domains and viewpoints for industrial-scale systems*  

Together, they provide a **complete architectural view** for CPS and Industrial IoT.

---

## 7. How to Use This Table

### For Education
- Teach CPS concepts using **5C**
- Introduce standards using **ISO/IEC 30141, RAMI 4.0, and IIRA**
- Show students how theory maps to real-world architectures

### For System Design
- Use **ISO/IEC 30141** for IoT layering
- Use **RAMI 4.0** for Industry 4.0 positioning
- Use **IIRA** for large-scale industrial systems
- Use **5C** to design closed-loop intelligence

---

## 8. Summary

- The **CPS 5C Architecture** is fully compatible with **ISO/IEC 30141**, **RAMI 4.0**, and **IIRA**
- Each framework addresses a **different architectural concern**
- The unified table provides a **clear, practical mapping**
- Together, they enable **standard-aligned, intelligent, and scalable CPS design**


