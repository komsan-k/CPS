# 🧪 Lab-0: Hello ChatGPT in Node-RED  
**Building Your First AI-Enabled Flow**

## 🎯 Lab Objective

In this lab, students will learn how to integrate ChatGPT with Node-RED using OpenAI’s modern Responses API.

By the end of this lab, you will be able to:
- Build a simple Node-RED flow that talks to ChatGPT
- Send a prompt from Node-RED to the OpenAI API
- Receive and display AI-generated responses
- Understand how AI fits into Cyber-Physical Systems (CPS) cognition layers

## 🧠 Why This Lab Matters (CPS Context)

In CPS and Digital Twin architectures:
- Sensors → Perception
- Data processing → Cyber
- ChatGPT → Cognition
- Human or controller → Decision / Configuration

This lab introduces AI as a cognition block, not as a low-level controller.

## 🧩 System Overview

Inject / User Input  
→ Function Node (Build prompt)  
→ HTTP Request (OpenAI API)  
→ Function Node (Extract response)  
→ Debug / Dashboard

## 🛠️ Requirements

### Software
- Node.js (LTS recommended)
- Node-RED
- Internet connection

### Accounts
- OpenAI API key

## 🚀 Step-by-Step Instructions

### Step 1 — Start Node-RED
```bash
node-red
```

Open http://localhost:1880

### Step 2 — Nodes Used
- Inject
- Function (Build OpenAI Request)
- HTTP Request
- Function (Extract AI Response)
- Debug

### Step 3 — Inject Payload
Explain Cyber-Physical Systems in 3 bullet points.

### Step 4 — Build OpenAI Request (Function Node)
```javascript
msg.headers = {
  "Authorization": "Bearer YOUR_API_KEY",
  "Content-Type": "application/json"
};

msg.payload = {
  model: "gpt-4.1-mini",
  input: msg.payload
};

return msg;
```

### Step 5 — HTTP Request Node
- Method: POST  
- URL: https://api.openai.com/v1/responses  
- Return: parsed JSON

### Step 6 — Extract AI Response (Function Node)
```javascript
msg.payload = msg.payload.output[0].content[0].text;
return msg;
```

### Step 7 — Run
Deploy → Click Inject → View response in Debug panel.

## 📊 Evaluation (10 Marks)
- Flow runs correctly: 4
- Correct API usage: 2
- Meaningful prompt: 2
- CPS explanation: 2

## 🧠 Key Takeaway
This lab demonstrates how AI cognition can be integrated into CPS using Node-RED.
