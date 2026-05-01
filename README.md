# 📁 Code Plagiarism Detector — Complete Project Guide

> A beginner-friendly C++ project made by Aryan Singh. Compiles with one command and generates both a terminal result and a beautiful HTML report.

---

## ✅ Verified Test Run Output

```
------------------------------------------------
       CODE PLAGIARISM DETECTOR
       Built in C++ | Beginner Friendly
------------------------------------------------

Enter name of File 1: file1.cpp
Enter name of File 2: file2.cpp

Reading files...
Files read successfully!

------------------------------------------------
 RESULTS
------------------------------------------------
 File 1         : file1.cpp
 File 2         : file2.cpp
 Lines in File 1: 16
 Lines in File 2: 20
 Matching Lines : 13
 Similarity     : 65%
 Verdict        : MODERATE SIMILARITY
------------------------------------------------

 MATCHING LINES:
------------------------------------------------
 [1] #include <iostream>
 [2] using namespace std;
 [3] int main() {
 [4] int a = 10;
 [5] int b = 20;
 [6] int sum = a + b;
 [7] cout << "sum = " << sum << endl;
 [8] return 0;
 ... (13 total)
```

---

## 📂 Project File Structure

```
Code Plagiarism Detector in C++/
│
├── main.cpp          ← The entire program (one file)
├── file1.cpp         ← Sample test file 1
├── file2.cpp         ← Sample test file 2 (partially copied)
├── build_and_run.bat ← Double-click to compile & run on Windows
└── result.html       ← Auto-generated after running the program
```

---

## 🔨 How to Compile & Run

### Option A — One Click (Recommended)
Double-click `build_and_run.bat` — it compiles and runs automatically.

### Option B — Manual (Terminal / CMD)
```bash
# Step 1: Go to the folder
cd "Code Plagiarism Detector in C++"

# Step 2: Compile
g++ main.cpp -o detector -std=c++11

# Step 3: Run
detector.exe
```

---

## 🧠 Code Explanation — Simple Words

### 📌 1. `#include` Headers
```cpp
#include <iostream>   // for cin, cout (input/output)
#include <fstream>    // for reading and writing files
#include <vector>     // for storing lists of lines
#include <string>     // for working with text
#include <algorithm>  // for transform() — lowercase conversion
```
These are the **toolboxes** C++ needs. We only use basic, standard ones.

---

### 📌 2. `cleanLine()` — The Cleaner Function
```cpp
string cleanLine(const string& line)
```
**What it does:**
- Removes extra spaces at the start/end of a line
- Collapses multiple spaces into one (so `"int   x"` becomes `"int x"`)
- Converts everything to **lowercase** (so `"Int"` and `"int"` are treated the same)

**Why we need it:**  
Without cleaning, two identical lines with different spacing would be counted as different. This makes the comparison **fair and accurate**.

---

### 📌 3. `readFile()` — The File Reader
```cpp
bool readFile(const string& filename, vector<string>& lines)
```
**What it does:**
- Opens the file using `ifstream`
- Reads it **line by line** using `getline()`
- Cleans each line with `cleanLine()`
- Stores non-empty lines in the `lines` vector
- Returns `true` if successful, `false` if the file doesn't exist

**Why use a vector?**  
A `vector<string>` is like a list that can grow — perfect for storing an unknown number of lines.

---

### 📌 4. `compareFiles()` — The Brain
```cpp
double compareFiles(
    const vector<string>& file1Lines,
    const vector<string>& file2Lines,
    vector<string>& matches
)
```
**What it does:**
- Loops through every line in File 1
- For each line, checks if the same line exists anywhere in File 2
- If a match is found → count it + save it in `matches`
- After all comparisons, calculates the **similarity %**

**The formula:**
```
similarity = (number of matching lines / max(total lines in file1, file2)) × 100
```

**Example:**
- File 1 has 16 lines, File 2 has 20 lines → max = 20
- 13 lines match → similarity = (13/20) × 100 = **65%**

---

### 📌 5. `escapeHTML()` — Safety for the Web
```cpp
string escapeHTML(const string& text)
```
**What it does:**  
Converts characters like `<`, `>`, `&` into their HTML-safe versions (`&lt;`, `&gt;`, `&amp;`).

**Why?**  
If a matching line contains `#include <iostream>`, the browser would treat `<iostream>` as an HTML tag and break the page. This function prevents that.

---

### 📌 6. `generateHTML()` — The Report Writer
```cpp
void generateHTML(...)
```
**What it does:**
- Opens (or creates) `result.html` using `ofstream`
- Writes a full HTML page with:
  - A **gradient title**
  - Two **file info cards**
  - A large **similarity % number** in color (green/yellow/orange/red)
  - A **progress bar**
  - A **table** of all matching lines
- Closes the file

**No external libraries** — pure `ofstream` + string writing.

---

### 📌 7. `getSeverityLabel()` / `getSeverityColor()`
These two small helper functions return a label and color based on the similarity score:

| Similarity | Label | Color |
|---|---|---|
| ≥ 80% | HIGH PLAGIARISM | 🔴 Red |
| 50–79% | MODERATE SIMILARITY | 🟠 Orange |
| 20–49% | LOW SIMILARITY | 🟡 Yellow |
| < 20% | LIKELY ORIGINAL | 🟢 Green |

---

### 📌 8. `main()` — The Coordinator
The `main()` function ties everything together:
1. Shows the title banner
2. Asks user for two file names
3. Calls `readFile()` for both
4. Calls `compareFiles()` to get results
5. Prints results to the terminal
6. Calls `generateHTML()` to create the report
7. Calls `system("start result.html")` to open it in the browser

---

## 🎨 HTML Report Design

The generated `result.html` has:
- **Dark background** (`#0d0d0d`)
- **Gradient title** (cyan → purple)
- **Color-coded similarity score** (changes based on %)
- **Animated progress bar**
- **Matching lines table** (monospace font, green text)
- Works in any browser with no internet required

---

## 🚀 How to Test With Your Own Files

1. Put your `.cpp` files in the **same folder** as `detector.exe`
2. Run the program
3. Type in the file names when asked
4. View results in terminal + browser

> [!TIP]
> You can also test `.txt`, `.py`, or `.java` files — the detector works on **any plain text file**.

---

## ⚠️ Error Handling Built In

| Situation | What happens |
|---|---|
| File doesn't exist | Shows `[ERROR] Could not open file: ...` and exits cleanly |
| Both files are empty | Returns 0% similarity (no division by zero) |
| No matching lines | Shows "Files appear to be original" in both terminal and HTML |
| `result.html` can't be created | Shows error message, doesn't crash |

---

## 📝 Key Concepts Used (Beginner Summary)

| Concept | Used For |
|---|---|
| `ifstream` | Reading files |
| `ofstream` | Writing the HTML file |
| `vector<string>` | Storing lines of each file |
| `getline()` | Reading one line at a time |
| `transform()` | Converting to lowercase |
| Nested `for` loops | Comparing every line pair |
| `system()` | Auto-opening the HTML file |

---

*Built with standard C++11 — no external libraries required.*
