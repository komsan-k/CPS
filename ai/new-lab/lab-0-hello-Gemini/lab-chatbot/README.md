# 🧪 Lab-0: Hello Gemini in Node-RED
**Building Your First Generative-AI Flow (Gemini Edition)**

---

## 🎯 Lab Objective

This lab introduces students to **Generative AI integration in Node-RED** using **Google Gemini**.
Students will learn how to send prompts from Node-RED to a cloud AI service and receive intelligent responses.

By the end of this lab, you will be able to:

- Build a Node-RED flow that connects to Gemini
- Send a natural-language prompt to a cloud AI model
- Parse and display AI-generated responses
- Understand the role of Generative AI as the **Cognition layer** in Cyber-Physical Systems (CPS)

---

## 🧠 CPS Context (Why This Lab Matters)

In CPS and Digital Twin systems:

- **Physical layer** → sensors and actuators
- **Cyber layer** → data processing and integration
- **Cognition layer** → AI-based reasoning and explanation
- **Configuration layer** → human or automated decisions

This lab focuses on **AI as cognition**, not as a real-time controller.

---

## 🧩 System Overview

```
Inject / User Input
        ↓
 Function Node
 (Build Gemini request)
        ↓
 HTTP Request
 (Gemini API)
        ↓
 Function Node
 (Extract response)
        ↓
 Debug / Dashboard
```

---

## 🛠️ Requirements

### Software
- Node.js (LTS recommended)
- Node-RED
- Internet connection

### Account
- Google account
- Gemini API key (Google AI Studio)

---

## 🔑 Getting a Gemini API Key

1. Go to: https://aistudio.google.com/app/apikey
2. Sign in with your Google account
3. Click **Create API key**
4. Copy the key (you cannot view it again)

⚠️ Keep your API key secret. Do not upload it to GitHub.

---

## 🚀 Step-by-Step Instructions

### Step 1 — Start Node-RED

```bash
node-red
```

Open your browser:
```
http://localhost:1880
```

---

### Step 2 — Create the Flow

Add the following nodes:

1. **Inject**
2. **Function** → *Build Gemini Request*
3. **HTTP Request**
4. **Function** → *Extract Gemini Response*
5. **Debug**

Wire them from left to right.

---

### Step 3 — Inject Node Configuration

Payload type: `string`

Example prompt:
```
Explain Cyber-Physical Systems in 3 bullet points.
```

---

### Step 4 — Function Node: Build Gemini Request

Name: **Build Gemini Request**

```javascript
const apiKey = "YOUR_GEMINI_API_KEY_HERE";
const model = "gemini-1.5-flash";

msg.method = "POST";
msg.url =
  `https://generativelanguage.googleapis.com/v1beta/models/${model}:generateContent?key=${apiKey}`;

msg.headers = {
  "Content-Type": "application/json"
};

msg.payload = {
  contents: [
    {
      parts: [
        { text: msg.payload }
      ]
    }
  ]
};

return msg;
```

---

### Step 5 — HTTP Request Node Configuration

- Method: **POST**
- URL: *(leave empty – set by Function node)*
- Return: **parsed JSON object**

---

### Step 6 — Function Node: Extract Gemini Response

Name: **Extract Gemini Response**

```javascript
try {
  msg.payload =
    msg.payload.candidates[0].content.parts[0].text;
} catch (e) {
  msg.payload = "Error parsing Gemini response";
}
return msg;
```

---

### Step 7 — Run the Flow

- Click **Deploy**
- Click **Inject**
- View Gemini’s response in the **Debug** panel

🎉 You have successfully connected Node-RED to Gemini!

---

## ✅ Expected Output (Example)

```
• CPS integrates computation with physical processes.
• Sensors and actuators form closed feedback loops.
• CPS enables intelligent systems such as smart grids and ITS.
```

---

## 🧪 Lab Tasks

| Task | Description |
|----|------------|
| 1 | Run the Gemini flow successfully |
| 2 | Modify the prompt |
| 3 | Observe response changes |
| 4 | Explain Gemini’s role in CPS |

---

## 📊 Evaluation (10 Marks)

| Component | Marks |
|--------|------|
| Flow runs correctly | 4 |
| Correct API usage | 2 |
| Meaningful prompt | 2 |
| CPS explanation | 2 |
| **Total** | **10** |

---

## 🔐 Security Notes

- Never publish API keys
- Do not hard-code keys in public repositories
- Prefer environment variables for advanced labs

---

## 🚀 Extensions (Optional)

- Replace Inject with **Dashboard text input**
- Compare **Gemini vs ChatGPT** in Node-RED
- Send Gemini output to **MQTT**
- Use Gemini to explain **Digital Twin state**

---

## 🧠 Key Takeaway

> Generative AI services like Gemini can be cleanly integrated into Node-RED and CPS as a **cloud-based cognition layer**.

---

**File purpose:**  
This README is intended as a complete, standalone **Lab-0** for CPS, IoT, and AI-enabled systems courses.

