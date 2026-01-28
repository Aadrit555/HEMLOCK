cat > README.md << 'EOF'
# Hemlock 🌿
**Media Integrity & Tamper Detection for Images and Videos**

Hemlock is a tool that helps you **prove whether an image or video has been changed after it was created**.

If a file is modified — even slightly — Hemlock will **detect it**, **explain it**, and **show where it happened**.

---

## 📌 What Problem Does Hemlock Solve?

Images and videos can be:
- edited
- cropped
- re-encoded
- partially changed
- or maliciously manipulated

After that, it becomes very hard to answer a simple question:

> **“Is this file still the original one?”**

Hemlock answers that question reliably.

---

## ✅ What Hemlock Can Do

- Detect **any change** in an image or video  
- Catch changes as small as **one pixel or one byte**  
- Verify files using **public-key cryptography**  
- Show **which video frame** was modified  
- Generate a **JSON verification report**  
- Produce a **visual image** of the tampered video frame  

---

## 🧠 How It Works (Simple Explanation)

### Images
Hemlock creates a strong digital fingerprint of the image and signs it.
If even one pixel changes, verification fails.

### Videos
A video is treated as many images (frames).
Hemlock fingerprints each frame, links them together, and signs the result.
If any frame is changed, removed, or reordered, verification fails.

---

## 🔐 Why This Is Secure

- **Private key** → used only while signing  
- **Public key** → used during verification  

This guarantees:
- authenticity
- integrity
- tamper detection

Anyone can verify the file, but no one can fake a valid signature.

---

## 🛠️ Technologies Used

- C — fast image processing  
- Python — video frame handling  
- ECDSA cryptography — signing & verification  
- JSON reports — machine-readable results  

---

## 🧪 Typical Use Cases

- Media authenticity checks  
- Digital forensics  
- ML dataset verification  
- Academic and security research  

---

### One-line summary

> **Hemlock detects and explains any tampering in images or videos using cryptographic verification.**
EOF