# Lab 0: “Hello ChatGPT” in Node-RED (Dashboard Chatbot)

## 🎯 Goal

Build a simple ChatGPT chatbot using Node-RED with the following flow:

**Text input (Dashboard) → Build prompt → HTTP Request (OpenAI Responses API) → Show reply (Dashboard + Debug)**

> OpenAI’s recommended modern endpoint is the **Responses API**.

---

## 🧰 What You Need

- Node-RED running on PC or Raspberry Pi  
- Node-RED Dashboard installed (optional but recommended)  
- An OpenAI API key  

---

## Step 1 — Install Dashboard Nodes (Optional)

In Node-RED:

1. Menu → **Manage palette** → **Install**  
2. Search for and install:  
   ```
   node-red-dashboard
   ```

> If you skip Dashboard, you can still use **Inject** nodes + **Debug** nodes instead.

---

## Step 2 — Create the Flow (Node List)

Add the following nodes:

1. **ui_text_input** — Dashboard input  
2. **function** — Build OpenAI request  
3. **http request** — Call OpenAI Responses API  
4. **function** — Extract response text  
5. **ui_text** — Display answer  
6. **debug** — View raw JSON response  

---

## Step 3 — Configure the HTTP Request Node

Set the **HTTP Request** node to:

- **Method:** `POST`  
- **URL:**  
  ```
  https://api.openai.com/v1/responses
  ```
- **Return:** Parsed JSON object  

---

## Step 4 — Function Node: “Build Request”

Add a **Function** node *before* the HTTP Request and paste the following code:

```javascript
// Takes msg.payload from the dashboard text input (a string)
const userText = (msg.payload || "").toString().trim();
if (!userText) return null;

// Put your API key here for the lab
// (Later you should move it to environment variables)
const OPENAI_KEY = "PASTE_YOUR_OPENAI_API_KEY_HERE";

msg.method = "POST";
msg.url = "https://api.openai.com/v1/responses";

msg.headers = {
  "Authorization": "Bearer " + OPENAI_KEY,
  "Content-Type": "application/json"
};

msg.payload = {
  model: "gpt-5-mini",
  input: `You are a helpful tutor. Answer in 3 bullet points.\n\nQuestion: ${userText}`,
  store: false
};

return msg;
```

📌 Notes:
- Node-RED HTTP Request nodes accept headers via `msg.headers`
- The Responses API uses the `/v1/responses` endpoint

---

## Step 5 — Function Node: “Extract Reply Text”

Add another **Function** node *after* the HTTP Request:

```javascript
const body = msg.payload || {};

// Try the simplest field first
let text = body.output_text;

// Fallback for nested shapes
if (!text && Array.isArray(body.output) && body.output[0]?.content?.[0]?.text) {
  text = body.output[0].content[0].text;
}

msg.payload = text || "[No text found in response]";
return msg;
```

> The Responses API JSON structure can vary; this extractor is intentionally defensive.

---

## Step 6 — Build the Dashboard UI

Add the following UI nodes:

- **ui_text_input**  
  - Label: `Ask ChatGPT`

- **ui_text**  
  - Label: `Answer`

Deploy the flow, then open:

```
http://<your-node-red-host>:1880/ui
```

---

## 👀 What Students Should Observe

- They type a question into the dashboard  
- ChatGPT returns a short answer  
- The Debug node shows the full JSON response from OpenAI  

---

## 📝 Mini Assignments (10–20 Minutes)

1. Change the system instruction to:  
   > “Answer like a CPS engineer.”

2. Add a **creativity / temperature** slider (optional exploration)

3. Add a mode toggle:
   - “Summarize in 1 sentence”
   - “Explain in bullet points”

---

## ⚠️ Safety Rule for Future CPS Labs (Important)

When you later connect sensors and actuators:

- **Node-RED rules control actuators**
- **ChatGPT only explains or summarizes**

✅ This keeps the CPS **deterministic, safe, and explainable**.
