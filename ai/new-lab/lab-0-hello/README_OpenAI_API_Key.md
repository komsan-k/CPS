# 🔑 How to Get YOUR_API_KEY_HERE (OpenAI API Key)
**Lab-0 Companion Guide for “Hello ChatGPT in Node-RED”**

This README explains how to obtain and safely use your OpenAI API key for Node-RED labs.

---

## ❓ What Is `YOUR_API_KEY_HERE`?

`YOUR_API_KEY_HERE` is a **personal OpenAI API key** used by Node-RED to authenticate requests to ChatGPT (OpenAI Responses API).

Think of it as:
- 🔐 A password for AI access
- 👤 Linked to your OpenAI account
- ⚠️ Secret information (must be protected)

---

## 🧭 Step-by-Step: Get Your OpenAI API Key

### 1️⃣ Log in to OpenAI
Go to:
https://platform.openai.com/

Log in using your OpenAI account.

---

### 2️⃣ Open the API Keys Page
Direct link:
https://platform.openai.com/api-keys

You will see the **API Keys** management page.

---

### 3️⃣ Create a New Secret Key
Click:

**➕ Create new secret key**

- Name example:
  ```
  Node-RED Lab-0
  ```
- Click **Create**

---

### 4️⃣ Copy the Key (IMPORTANT)
You will see a key similar to:

```
sk-XXXXXXXXXXXXXXXXXXXXXXXXXXXX
```

✅ Copy it immediately  
❌ You cannot view it again later

---

## 🧩 Using the Key in Node-RED (Lab-0)

Open the **Function node: “Build OpenAI Request”** and replace:

```javascript
const apiKey = "YOUR_API_KEY_HERE";
```

with:

```javascript
const apiKey = "sk-XXXXXXXXXXXXXXXXXXXXXXXXXXXX";
```

✔ Keep the quotation marks  
✔ No spaces  
❌ Do not commit this to GitHub

---

## 🔐 Security Rules (Very Important)

### ❌ Do NOT
- Share your API key with others
- Upload flows containing keys to GitHub
- Include keys in screenshots or reports

### ✅ Do
- Rotate keys if compromised
- Use environment variables (recommended)
- Treat keys like passwords

---

## ⭐ Recommended Professional Method (Environment Variables)

### Windows (PowerShell)
```powershell
setx OPENAI_API_KEY "sk-XXXXXXXXXXXXXXXX"
```

Restart Node-RED after running this.

---

### macOS / Linux
```bash
export OPENAI_API_KEY="sk-XXXXXXXXXXXXXXXX"
```

---

### Use Environment Variable in Node-RED Function
```javascript
const apiKey = process.env.OPENAI_API_KEY;
```

✅ No hard-coded secrets  
✅ Safe for labs and classrooms  
✅ Industry best practice

---

## 🧪 Troubleshooting

| Problem | Cause | Solution |
|------|------|--------|
| 401 Unauthorized | Invalid or missing API key | Check key value |
| No response | Network / rate limit | Retry later |
| Empty payload | Parsing error | Check Extract function |

---

## 🎓 Instructor Notes

- Each student should use **their own API key**
- Explain **cost awareness and rate limits**
- Encourage environment-variable usage from Lab-0

---

## 🧠 Key Takeaway

> API keys enable secure AI access.  
> Proper key management is essential in CPS, IoT, and AI-enabled systems.

---

**File purpose:**  
This README is designed to be bundled with **Lab-0: Hello ChatGPT in Node-RED** as a security and setup guide.

