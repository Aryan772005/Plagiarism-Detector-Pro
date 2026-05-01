# 📘 How to Use — Code Plagiarism Detector
### (Simple Guide for Non-Tech Users)

---

## ✅ PART 1 — VS Code Extension (Recommended — No Commands Needed!)

### Step 1: Install the Extension (ONE TIME ONLY)
> Already done! The extension is installed in your VS Code.

If you need to reinstall:
1. Open the `vscode-extension` folder
2. Double-click **`install.bat`**
3. Restart VS Code

---

### Step 2: Restart VS Code
Close VS Code completely and reopen it.
You'll see a message at the bottom-right: **"🔍 Code Plagiarism Detector is active!"**

---

### Step 3: It Works Automatically!

| What You Do | What Happens Automatically |
|---|---|
| **Open 2 or more code files** | Extension reads them all |
| **Press Ctrl+S to save** | Instantly checks similarity with all open files |
| **> 50% similarity found** | Orange warning pops up |
| **Status bar (bottom)** | Shows similarity % live |

---

### Step 4: Open the Dashboard
Press **`Ctrl+Shift+P`** → type **`Plagiarism Detector: Open Dashboard`** → Press Enter

The dashboard shows:
- 📁 **File Analysis tab** — Lines, Functions, Comments, Quality Score for every open file
- 📊 **Similarity Report tab** — Side-by-side comparison of all file pairs with % and matching lines
- 📖 **How to Use tab** — In-app guide

---

### Step 5: Beautify Messy Code
1. Open any messy `.cpp` or code file
2. **Right-click** anywhere in the code
3. Click **"Plagiarism Detector: Beautify Current File"**
4. Your code is instantly cleaned and properly indented ✨

---

### Step 6: See Code Statistics
Right-click → **"Plagiarism Detector: Show Code Statistics"**

Shows:
- Total lines, code lines, blank lines
- Number of functions, classes, loops
- Comment count
- **Code Quality Score (0–100%)**
- Beautified preview of your code

---

### Step 7 (Optional): Turn ON Auto-Beautify on Save
1. Go to **File → Preferences → Settings** (or press `Ctrl+,`)
2. Search for: **`Plagiarism`**
3. Check the box: **"Auto Beautify On Save"**
4. Now every time you save, your code is auto-formatted!

---

## ✅ PART 2 — C++ Terminal Program (No VS Code needed)

### How to Run
1. Open the `Code Plagiarism Detector in C++` folder
2. Double-click **`build_and_run.bat`**
3. A black terminal window opens

### Menu Options
```
1. Compare Files    → Type file names, see similarity %
2. Beautify Code    → Type a file name, get clean code
3. Exit
```

### Example: Compare file1.cpp and file2.cpp
```
How many files? → type: 2
File 1 name?    → type: file1.cpp
File 2 name?    → type: file2.cpp
```
Results appear in the terminal AND a **result.html** report opens in your browser automatically!

---

## 🔍 What "Similarity %" Means

| % Range | Meaning | Color |
|:---:|---|:---:|
| 80–100% | HIGH PLAGIARISM — almost identical code | 🔴 Red |
| 50–79%  | MODERATE — significant overlap | 🟠 Orange |
| 20–49%  | LOW — some shared lines | 🟡 Yellow |
| 0–19%   | LIKELY ORIGINAL — mostly unique | 🟢 Green |

---

## 📂 What Each File Does

| File | What it is |
|---|---|
| `main.cpp` | The C++ program source code |
| `build_and_run.bat` | Double-click to compile and run |
| `file1.cpp` / `file2.cpp` | Sample test files |
| `messy.cpp` | Sample messy code to test beautifier |
| `result.html` | Auto-generated report (opens in browser) |
| `formatted_messy.cpp` | Beautified version of messy.cpp |
| `vscode-extension/` | The VS Code extension folder |

---

*Built for Final Year Capstone Project — C++ Plagiarism Detector + Code Beautifier*
