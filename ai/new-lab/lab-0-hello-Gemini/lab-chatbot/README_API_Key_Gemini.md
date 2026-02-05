# 🔑 README_API_Key_Gemini.md
**How to Get and Use a Gemini API Key (for Node-RED Labs)**

This guide explains how students can obtain and safely use a **Google Gemini API key** for **Lab-0: Hello Gemini in Node-RED**.

---

## ❓ What Is a Gemini API Key?

A Gemini API key is a **secret credential** that allows your Node-RED flow to access Google’s Generative AI (Gemini) service.

Think of it as:
- 🔐 A password for AI access
- ☁️ Permission to use cloud-based cognition
- ⚠️ Confidential information (must be protected)

---

## 🧭 Step-by-Step: Get Your Gemini API Key

### 1️⃣ Open Google AI Studio
Go to:
👉 https://aistudio.google.com/app/apikey

Log in using your **Google account**.

---

### 2️⃣ Create a New API Key
Click:

**➕ Create API key**

You may see a warning about usage limits — this is normal.

---

### 3️⃣ Copy the API Key (IMPORTANT)

Your key will look like:

```
AIzaSyXXXXXXXXXXXXXXXXXXXXXXXXXXX
```

✅ Copy it immediately  
❌ You cannot view it again later

---

## 🧩 Use the API Key in Node-RED (Lab-0)

Open the **Function node** named:

> **Build Gemini Request**

Replace:
```javascript
const apiKey = "YOUR_GEMINI_API_KEY_HERE";
```

with:
```javascript
const apiKey = "AIzaSyXXXXXXXXXXXXXXXXXXXXXXXXXXX";
```

✔ Keep quotation marks  
✔ No extra spaces  
❌ Never upload this key to GitHub

---

## ⭐ Recommended (Professional Method)

Instead of hard-coding the key, use an **environment variable**.

### Windows (PowerShell)
```powershell
setx GEMINI_API_KEY "AIzaSyXXXXXXXXXXXXXXXX"
```

Restart Node-RED after this step.

---

### macOS / Linux
```bash
export GEMINI_API_KEY="AIzaSyXXXXXXXXXXXXXXXX"
```

---

### Node-RED Function (Recommended)
```javascript
const apiKey = process.env.GEMINI_API_KEY;
```

✅ Safer  
✅ Cleaner  
✅ Industry best practice

---

## 🧪 Troubleshooting

| Issue | Meaning | Fix |
|----|--------|----|
| 403 / 401 error | Invalid or missing API key | Recheck key |
| Quota exceeded | Free quota exhausted | Wait or upgrade |
| Empty response | Parsing error | Check extract function |

---

## 🔐 Security Rules (Must Follow)

### ❌ Do NOT
- Share API keys with classmates
- Upload flows with keys to GitHub
- Include keys in screenshots or reports

### ✅ Do
- Rotate keys if leaked
- Use environment variables
- Treat API keys like passwords

---

## 🎓 Instructor Notes

- Each student should use **their own Gemini API key**
- Explain quota limits before running labs
- Encourage environment-variable usage from Lab-0

---

## 🧠 Key Takeaway

> Gemini API keys enable **cloud-based AI cognition**.  
> Secure key management is essential in CPS, IoT, and AI-enabled systems.

---

**File usage:**  
This file is designed to be bundled as:

```
README_API_Key_Gemini.md
```

alongside:
- `README.md` (Lab-0 instructions)
- `flow_lab0_gemini.json`
