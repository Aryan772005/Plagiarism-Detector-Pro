// ============================================================
//   CODE PLAGIARISM DETECTOR + CODE BEAUTIFIER
//   Enhanced Version — Single File (main.cpp)
//
//   Features:
//     1. Compare up to 10 files (all pairs)
//     2. Difference viewer
//     3. Dark-themed HTML report
//     4. Code Beautifier (state-machine parser)
//
//   Compile:  g++ main.cpp -o detector -std=c++11
//   Run:      detector.exe
// ============================================================

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>

using namespace std;

// ─────────────────────────────────────────
//  UTILITY HELPERS
// ─────────────────────────────────────────

string getBaseName(const string& path) {
    size_t pos = path.find_last_of("/\\");
    return (pos == string::npos) ? path : path.substr(pos + 1);
}

void printDivider(char c = '-', int n = 48) {
    for (int i = 0; i < n; i++) cout << c;
    cout << "\n";
}

// Trim leading/trailing whitespace
string trim(const string& s) {
    int a = 0, b = (int)s.size() - 1;
    while (a <= b && (s[a] == ' ' || s[a] == '\t' || s[a] == '\r')) a++;
    while (b >= a && (s[b] == ' ' || s[b] == '\t' || s[b] == '\r')) b--;
    return (a <= b) ? s.substr(a, b - a + 1) : "";
}

// Collapse multiple spaces into one + lowercase
string cleanLine(const string& line) {
    string r = "";
    bool lastSpace = false, started = false;
    for (char c : line) {
        if (c == ' ' || c == '\t') {
            if (started && !lastSpace) { r += ' '; lastSpace = true; }
        } else {
            r += (char)tolower(c);
            lastSpace = false;
            started = true;
        }
    }
    while (!r.empty() && r.back() == ' ') r.pop_back();
    return r;
}

// Escape special HTML chars so code renders safely in browser
string escapeHTML(const string& s) {
    string r = "";
    for (char c : s) {
        if      (c == '&')  r += "&amp;";
        else if (c == '<')  r += "&lt;";
        else if (c == '>')  r += "&gt;";
        else if (c == '"')  r += "&quot;";
        else                r += c;
    }
    return r;
}

// ─────────────────────────────────────────
//  READ FILE
// ─────────────────────────────────────────
bool readFile(const string& fn, vector<string>& lines) {
    ifstream f(fn);
    if (!f.is_open()) {
        cout << "\n  [ERROR] Cannot open: \"" << fn << "\"\n";
        cout << "          Make sure the file exists and the path is correct.\n";
        return false;
    }
    string line;
    while (getline(f, line)) {
        string cl = cleanLine(line);
        if (!cl.empty()) lines.push_back(cl);
    }
    return true;
}

// Read raw (unmodified) file content into one string
bool readRaw(const string& fn, string& content) {
    ifstream f(fn);
    if (!f.is_open()) {
        cout << "\n  [ERROR] Cannot open: \"" << fn << "\"\n";
        return false;
    }
    content = string((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
    return true;
}

// ─────────────────────────────────────────
//  COMPARE FILES
// ─────────────────────────────────────────
double compareFiles(const vector<string>& f1, const vector<string>& f2,
                    vector<string>& matches) {
    matches.clear();
    for (int i = 0; i < (int)f1.size(); i++)
        for (int j = 0; j < (int)f2.size(); j++)
            if (f1[i] == f2[j]) { matches.push_back(f1[i]); break; }

    int mx = max((int)f1.size(), (int)f2.size());
    return mx == 0 ? 0.0 : (double)matches.size() / mx * 100.0;
}

// ─────────────────────────────────────────
//  DIFFERENCE VIEWER
// ─────────────────────────────────────────
void showDifferences(const string& n1, const vector<string>& f1,
                     const string& n2, const vector<string>& f2) {
    cout << "\n"; printDivider();
    cout << "  DIFFERENCE VIEWER\n"; printDivider();

    auto printUnique = [&](const string& nameA, const vector<string>& A,
                           const string& nameB, const vector<string>& B) {
        cout << "\n  Lines in [" << nameA << "] but NOT in [" << nameB << "]:\n";
        bool any = false;
        for (auto& line : A) {
            bool found = false;
            for (auto& bl : B) if (line == bl) { found = true; break; }
            if (!found) { cout << "    >> " << line << "\n"; any = true; }
        }
        if (!any) cout << "    (none)\n";
    };

    printUnique(n1, f1, n2, f2);
    printUnique(n2, f2, n1, f1);
    cout << "\n"; printDivider();
}

// ─────────────────────────────────────────
//  SEVERITY HELPERS
// ─────────────────────────────────────────
string severityLabel(double s) {
    if (s >= 80) return "HIGH PLAGIARISM";
    if (s >= 50) return "MODERATE SIMILARITY";
    if (s >= 20) return "LOW SIMILARITY";
    return "LIKELY ORIGINAL";
}
string severityColor(double s) {
    if (s >= 80) return "#ff4d4d";
    if (s >= 50) return "#ff9800";
    if (s >= 20) return "#ffd700";
    return "#00e676";
}

// ─────────────────────────────────────────
//  HTML REPORT GENERATOR
// ─────────────────────────────────────────
struct PairResult {
    int i, j;
    double sim;
    vector<string> matches;
};

void generateHTML(const vector<string>& names,
                  const vector<vector<string>>& lines,
                  const vector<PairResult>& pairs,
                  int topIdx) {

    ofstream html("result.html");
    if (!html.is_open()) { cout << "  [ERROR] Cannot create result.html\n"; return; }

    // --- CSS ---
    html << R"(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1.0">
<title>Plagiarism Detector Report</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{background:#080810;color:#ddd;font-family:'Segoe UI',sans-serif;padding:40px 20px}
.wrap{max-width:960px;margin:0 auto}
h1{text-align:center;font-size:2.3rem;letter-spacing:2px;
   background:linear-gradient(90deg,#00c8ff,#a259ff);
   -webkit-background-clip:text;-webkit-text-fill-color:transparent;
   background-clip:text;margin-bottom:6px}
.sub{text-align:center;color:#555;font-size:.9rem;margin-bottom:34px}
.card{background:#111120;border:1px solid #1e1e3a;border-radius:14px;
      padding:24px 28px;margin-bottom:20px}
.card.top{border-color:#a259ff66;box-shadow:0 0 30px #a259ff1a}
.sec{font-size:.72rem;text-transform:uppercase;letter-spacing:2px;
     color:#555;margin-bottom:14px;padding-bottom:8px;border-bottom:1px solid #1e1e3a}
.grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(180px,1fr));gap:12px}
.stat{background:#0d0d1a;border-radius:8px;padding:12px 16px}
.stat b{display:block;font-size:1.15rem;color:#e0e0e0;margin-bottom:2px}
.stat span{font-size:.78rem;color:#666}
.pair-head{display:flex;justify-content:space-between;align-items:flex-start;
           flex-wrap:wrap;gap:12px;margin-bottom:14px}
.fnames{font-size:1rem;font-weight:600}
.fnames em{color:#00c8ff;font-style:normal}
.fnames .vs{color:#444;margin:0 6px}
.best-tag{background:#a259ff22;color:#a259ff;border:1px solid #a259ff55;
          border-radius:6px;padding:2px 10px;font-size:.7rem;
          font-weight:700;letter-spacing:1px;margin-left:8px;vertical-align:middle}
.pct{font-size:2.6rem;font-weight:800;line-height:1;text-align:right}
.verdict{font-size:.78rem;font-weight:700;letter-spacing:1.5px;text-align:right;margin-top:4px}
.bar-bg{background:#0d0d1a;border-radius:999px;height:10px;
        margin:14px 0 18px;overflow:hidden}
.bar-fill{height:100%;border-radius:999px}
table{width:100%;border-collapse:collapse;font-size:.82rem}
th{text-align:left;padding:9px 12px;background:#0d0d1a;color:#666;letter-spacing:.5px}
td{padding:8px 12px;border-top:1px solid #1a1a2e;
   font-family:'Courier New',monospace;color:#a8ffa8;word-break:break-all}
tr:hover td{background:#0d0d1a}
.empty{color:#444;font-style:italic;padding:16px 0;text-align:center}
.summary-grid{display:grid;grid-template-columns:repeat(3,1fr);gap:14px}
footer{text-align:center;color:#252535;font-size:.75rem;margin-top:36px}
</style></head><body><div class="wrap">
)";

    // --- Header ---
    html << "<h1>&#128269; Code Plagiarism Detector</h1>\n";
    html << "<p class=\"sub\">" << names.size()
         << " file(s) analyzed &mdash; " << pairs.size() << " pair(s) compared</p>\n";

    // --- Summary stats card ---
    double totalSim = 0, maxSim = pairs[0].sim, minSim = pairs[0].sim;
    for (auto& p : pairs) {
        totalSim += p.sim;
        maxSim = max(maxSim, p.sim);
        minSim = min(minSim, p.sim);
    }
    double avgSim = totalSim / pairs.size();

    html << "<div class=\"card\"><div class=\"sec\">&#128202; Summary Statistics</div>\n";
    html << "<div class=\"summary-grid\">\n";
    html << "<div class=\"stat\"><b>" << (int)maxSim << "%</b><span>Highest Similarity</span></div>\n";
    html << "<div class=\"stat\"><b>" << (int)avgSim << "%</b><span>Average Similarity</span></div>\n";
    html << "<div class=\"stat\"><b>" << (int)minSim << "%</b><span>Lowest Similarity</span></div>\n";
    html << "</div></div>\n";

    // --- Files card ---
    html << "<div class=\"card\"><div class=\"sec\">&#128196; Files Analyzed</div>\n<div class=\"grid\">\n";
    for (int k = 0; k < (int)names.size(); k++) {
        html << "<div class=\"stat\"><b>" << escapeHTML(names[k]) << "</b>"
             << "<span>" << lines[k].size() << " lines</span></div>\n";
    }
    html << "</div></div>\n";

    // --- Pair cards ---
    for (int p = 0; p < (int)pairs.size(); p++) {
        auto& pr = pairs[p];
        string col = severityColor(pr.sim);
        string lbl = severityLabel(pr.sim);
        bool isTop = (p == topIdx);

        html << "<div class=\"card" << (isTop ? " top" : "") << "\">\n";
        html << "<div class=\"pair-head\">\n";
        html << "<div class=\"fnames\"><em>" << escapeHTML(names[pr.i]) << "</em>";
        html << "<span class=\"vs\">vs</span><em>" << escapeHTML(names[pr.j]) << "</em>";
        if (isTop) html << "<span class=\"best-tag\">&#11088; MOST SIMILAR</span>";
        html << "</div>\n";
        html << "<div><div class=\"pct\" style=\"color:" << col << "\">"
             << (int)pr.sim << "<span style=\"font-size:1.3rem\">%</span></div>\n";
        html << "<div class=\"verdict\" style=\"color:" << col << "\">" << lbl << "</div></div>\n</div>\n";

        // Stats row
        html << "<div class=\"grid\" style=\"margin-bottom:4px\">\n";
        html << "<div class=\"stat\"><b>" << lines[pr.i].size() << "</b><span>Lines in file 1</span></div>\n";
        html << "<div class=\"stat\"><b>" << lines[pr.j].size() << "</b><span>Lines in file 2</span></div>\n";
        html << "<div class=\"stat\"><b>" << pr.matches.size() << "</b><span>Matching lines</span></div>\n";
        html << "</div>\n";

        // Progress bar
        html << "<div class=\"bar-bg\"><div class=\"bar-fill\" style=\"width:" << (int)pr.sim
             << "%;background:linear-gradient(90deg," << col << "55," << col << ")\"></div></div>\n";

        // Matching lines table
        html << "<div class=\"sec\">&#9989; Matching Lines</div>\n";
        if (pr.matches.empty()) {
            html << "<div class=\"empty\">No matching lines found.</div>\n";
        } else {
            html << "<table><thead><tr><th>#</th><th>Matched Line</th></tr></thead><tbody>\n";
            for (int m = 0; m < (int)pr.matches.size(); m++)
                html << "<tr><td>" << (m+1) << "</td><td>"
                     << escapeHTML(pr.matches[m]) << "</td></tr>\n";
            html << "</tbody></table>\n";
        }
        html << "</div>\n";
    }

    html << "<footer>Code Plagiarism Detector &mdash; C++ Capstone Project</footer>\n";
    html << "</div></body></html>\n";
    html.close();
}

// ─────────────────────────────────────────
//  CODE BEAUTIFIER (State-Machine Parser)
//
//  Handles:
//    - String literals  ("...")
//    - Char literals    ('...')
//    - Line comments    (// ...)
//    - Block comments   (/* ... */)
//    - Preprocessor     (#include, #define ...)
//    - Keyword spacing  if( → if (
//    - Auto-indentation with 4-space indent
// ─────────────────────────────────────────

// Add a space before '(' after keywords
string addKeywordSpacing(const string& code) {
    vector<string> kw = {"if", "for", "while", "switch", "else", "catch"};
    string r = code;
    for (auto& k : kw) {
        string from = k + "(";
        string to   = k + " (";
        size_t pos = 0;
        while ((pos = r.find(from, pos)) != string::npos) {
            // Make sure it's not e.g. "xif("
            bool ok = (pos == 0) || !isalnum(r[pos-1]);
            if (ok) { r.replace(pos, from.size(), to); pos += to.size(); }
            else    { pos++; }
        }
    }
    return r;
}

string beautifyCode(const string& raw, bool removeComments = false) {
    // States
    enum { NORMAL, IN_STR, IN_CHAR, IN_LINE_CMT, IN_BLOCK_CMT } state = NORMAL;

    int indent = 0;
    const string IND = "    "; // 4 spaces

    auto getInd = [&]() { string s; for (int k=0;k<indent;k++) s+=IND; return s; };

    vector<string> outLines;
    string cur = "";      // current token being assembled
    bool prevWasClose = false; // was last real line a }?

    // Flush 'cur' as a formatted code line
    auto flushCur = [&]() {
        string t = trim(cur);
        cur = "";
        if (t.empty()) return;
        t = addKeywordSpacing(t);
        outLines.push_back(getInd() + t);
        prevWasClose = false;
    };

    int n = (int)raw.size();
    for (int i = 0; i < n; i++) {
        char c    = raw[i];
        char nxt  = (i+1 < n) ? raw[i+1] : '\0';

        // ── In line comment ──────────────────────
        if (state == IN_LINE_CMT) {
            if (c == '\n') {
                string t = trim(cur); cur = "";
                if (!t.empty() && !removeComments) outLines.push_back(getInd() + "// " + t);
                state = NORMAL;
            } else { cur += c; }
            continue;
        }

        // ── In block comment ─────────────────────
        if (state == IN_BLOCK_CMT) {
            if (c == '*' && nxt == '/') {
                string t = trim(cur); cur = "";
                if (!removeComments) outLines.push_back(getInd() + "/* " + t + " */");
                state = NORMAL; i++;
            } else { if (c != '\n' && c != '\r') cur += c; }
            continue;
        }

        // ── In string literal ────────────────────
        if (state == IN_STR) {
            cur += c;
            if (c == '\\' && i+1 < n) { cur += raw[++i]; }
            else if (c == '"') state = NORMAL;
            continue;
        }

        // ── In char literal ──────────────────────
        if (state == IN_CHAR) {
            cur += c;
            if (c == '\\' && i+1 < n) { cur += raw[++i]; }
            else if (c == '\'') state = NORMAL;
            continue;
        }

        // ── NORMAL state ─────────────────────────

        // Start line comment
        if (c == '/' && nxt == '/') {
            flushCur(); state = IN_LINE_CMT; i++; continue;
        }
        // Start block comment
        if (c == '/' && nxt == '*') {
            flushCur(); state = IN_BLOCK_CMT; i++; continue;
        }
        // Start string
        if (c == '"') { cur += c; state = IN_STR; continue; }
        // Start char
        if (c == '\'') { cur += c; state = IN_CHAR; continue; }

        // Preprocessor directive (# at start of logical line)
        if (c == '#' && trim(cur).empty()) {
            string directive = "#";
            while (i+1 < n && raw[i+1] != '\n') directive += raw[++i];
            outLines.push_back(directive);   // no indentation for preprocessor
            cur = ""; prevWasClose = false;
            continue;
        }

        // Open brace
        if (c == '{') {
            flushCur();
            outLines.push_back(getInd() + "{");
            indent++;
            continue;
        }

        // Close brace
        if (c == '}') {
            flushCur();
            if (indent > 0) indent--;
            outLines.push_back(getInd() + "}");
            prevWasClose = true;
            continue;
        }

        // Semicolon — ends a statement
        if (c == ';') {
            cur += ';';
            flushCur();
            continue;
        }

        // Newlines / carriage returns → treat as space
        if (c == '\n' || c == '\r') {
            if (!cur.empty() && cur.back() != ' ') cur += ' ';
            continue;
        }

        cur += c;
    }
    flushCur(); // flush anything remaining

    // Build final string
    string out = "";
    for (auto& l : outLines) out += l + "\n";
    return out;
}

// ─────────────────────────────────────────
//  FEATURE: COMPARE FILES MENU
// ─────────────────────────────────────────
void compareFilesMenu() {
    cout << "\n"; printDivider();
    cout << "  FILE COMPARISON\n"; printDivider();

    int numFiles = 0;
    cout << "  How many files to compare? (2 to 10): ";
    cin >> numFiles;
    if (numFiles < 2 || numFiles > 10) {
        cout << "  [ERROR] Enter a number between 2 and 10.\n"; return;
    }

    vector<string> names(numFiles);
    cout << "\n";
    for (int i = 0; i < numFiles; i++) {
        cout << "  Enter name (or full path) of File " << (i+1) << ": ";
        getline(cin >> ws, names[i]);
        if (names[i].size() >= 2 && names[i].front() == '"' && names[i].back() == '"') {
            names[i] = names[i].substr(1, names[i].size() - 2);
        }
    }

    cout << "\n  Reading files...\n";
    vector<vector<string>> fileLines(numFiles);
    for (int i = 0; i < numFiles; i++) {
        if (!readFile(names[i], fileLines[i])) { cout << "  Aborting.\n"; return; }
        names[i] = getBaseName(names[i]); // Shorten name for display
    }
    cout << "  All files read successfully!\n";

    // Compare every pair
    vector<PairResult> pairs;
    int topIdx = 0; double topSim = -1;

    for (int i = 0; i < numFiles; i++) {
        for (int j = i+1; j < numFiles; j++) {
            PairResult pr;
            pr.i = i; pr.j = j;
            pr.sim = compareFiles(fileLines[i], fileLines[j], pr.matches);
            pairs.push_back(pr);
            if (pr.sim > topSim) { topSim = pr.sim; topIdx = (int)pairs.size()-1; }
        }
    }

    // ── Terminal output ──
    cout << "\n"; printDivider();
    cout << "  RESULTS — " << pairs.size() << " pair(s) compared\n"; printDivider();

    for (int p = 0; p < (int)pairs.size(); p++) {
        auto& pr = pairs[p];
        cout << "\n  [Pair " << (p+1) << "]  " << names[pr.i] << "  vs  " << names[pr.j] << "\n";
        cout << "   Lines (File 1): " << fileLines[pr.i].size() << "\n";
        cout << "   Lines (File 2): " << fileLines[pr.j].size() << "\n";
        cout << "   Matching      : " << pr.matches.size() << "\n";
        cout << "   Similarity    : " << (int)pr.sim << "%\n";
        cout << "   Verdict       : " << severityLabel(pr.sim) << "\n";
        if (!pr.matches.empty()) {
            cout << "   Matching Lines:\n";
            for (int m = 0; m < (int)pr.matches.size(); m++)
                cout << "     [" << (m+1) << "] " << pr.matches[m] << "\n";
        }
    }

    // Summary stats
    double total = 0, hi = pairs[0].sim, lo = pairs[0].sim;
    for (auto& p : pairs) { total += p.sim; hi = max(hi, p.sim); lo = min(lo, p.sim); }
    double avg = total / pairs.size();

    cout << "\n"; printDivider('=');
    cout << "  SUMMARY\n"; printDivider('=');
    cout << "  Highest Similarity : " << (int)hi  << "%  ("
         << names[pairs[topIdx].i] << " vs " << names[pairs[topIdx].j] << ")\n";
    cout << "  Average Similarity : " << (int)avg << "%\n";
    cout << "  Lowest  Similarity : " << (int)lo  << "%\n";
    printDivider('=');

    // Show diff for most similar pair
    showDifferences(names[pairs[topIdx].i], fileLines[pairs[topIdx].i],
                    names[pairs[topIdx].j], fileLines[pairs[topIdx].j]);

    // HTML Report
    cout << "\n  Generating HTML report...\n";
    generateHTML(names, fileLines, pairs, topIdx);
    cout << "  Saved: result.html\n";
    cout << "  Opening in browser...\n";
    system("start result.html");
}

// ─────────────────────────────────────────
//  FEATURE: CODE BEAUTIFIER MENU
// ─────────────────────────────────────────
void beautifyMenu() {
    cout << "\n"; printDivider();
    cout << "  CODE BEAUTIFIER\n"; printDivider();

    string filename;
    cout << "  Enter file name (or full path) to beautify: ";
    getline(cin >> ws, filename);
    if (filename.size() >= 2 && filename.front() == '"' && filename.back() == '"') {
        filename = filename.substr(1, filename.size() - 2);
    }

    string raw;
    if (!readRaw(filename, raw)) return;

    cout << "  Remove comments? (y/n): ";
    char removeChoice;
    cin >> removeChoice;
    bool removeComments = (removeChoice == 'y' || removeChoice == 'Y');

    string formatted = beautifyCode(raw, removeComments);

    cout << "\n  ── Beautified Output ──\n\n";
    cout << formatted;

    printDivider();

    // Save to formatted_<filename>
    string outName = "formatted_" + getBaseName(filename);
    ofstream out(outName);
    if (out.is_open()) {
        out << formatted;
        out.close();
        cout << "\n  Saved to: " << outName << "\n";
    } else {
        cout << "\n  [ERROR] Could not save formatted file.\n";
    }
}

// ─────────────────────────────────────────
//  FEATURE: AI CODE ANALYSIS MENU
// ─────────────────────────────────────────
void codeAnalysisMenu() {
    cout << "\n"; printDivider();
    cout << "  AI CODE ANALYSIS\n"; printDivider();
    
    string filename;
    cout << "  Enter file name (or full path) to analyze: ";
    getline(cin >> ws, filename);
    if (filename.size() >= 2 && filename.front() == '"' && filename.back() == '"') {
        filename = filename.substr(1, filename.size() - 2);
    }
    
    string raw;
    if (!readRaw(filename, raw)) return;

    // Safeguard for Groq Free Tier TPM Limits
    if (raw.size() > 10000) {
        cout << "\n  [WARNING] File is very large. Truncating to fit free AI tier limits...\n";
        raw = raw.substr(0, 10000) + "\n\n... [CODE TRUNCATED DUE TO API LIMITS] ...";
    }

    string apiKey = "YOUR_GROQ_API_KEY_HERE";

    cout << "\n  Analyzing code with AI... Please wait...\n";

    // Write raw code to a temporary file
    ofstream codeOut("temp_groq_code.txt");
    codeOut << raw;
    codeOut.close();

    // Generate a PowerShell script to safely encode the JSON and make the request
    ofstream psOut("run_groq.ps1");
    psOut << "$ErrorActionPreference = 'Stop'\n";
    psOut << "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12\n";
    psOut << "$code = Get-Content '.\\temp_groq_code.txt' -Raw\n";
    psOut << "$headers = @{ 'Authorization' = 'Bearer " << apiKey << "'; 'Content-Type' = 'application/json; charset=utf-8' }\n";
    psOut << "$body = @{\n";
    psOut << "    model = 'llama-3.1-8b-instant'\n";
    psOut << "    messages = @(\n";
    psOut << "        @{\n";
    psOut << "            role = 'user'\n";
    psOut << "            content = \"You are a senior C++ developer. Analyze this code and provide a short summary of its quality, potential bugs, and time complexity. Keep it concise.`n`nCode:`n$code\"\n";
    psOut << "        }\n";
    psOut << "    )\n";
    psOut << "}\n";
    psOut << "$jsonBody = [System.Text.Encoding]::UTF8.GetBytes(($body | ConvertTo-Json -Depth 5))\n";
    psOut << "try {\n";
    psOut << "    $response = Invoke-RestMethod -Uri 'https://api.groq.com/openai/v1/chat/completions' -Method Post -Headers $headers -Body $jsonBody -ContentType 'application/json; charset=utf-8'\n";
    psOut << "    Write-Host $response.choices[0].message.content\n";
    psOut << "} catch {\n";
    psOut << "    Write-Host 'Error:' $_.Exception.Message\n";
    psOut << "    if ($_.ErrorDetails) { Write-Host $_.ErrorDetails.Message }\n";
    psOut << "}\n";
    psOut.close();

    cout << "\n  ── AI Analysis Result ──\n\n";
    system("powershell -ExecutionPolicy Bypass -NoProfile -File .\\run_groq.ps1");
    
    // Clean up temp files
    remove("temp_groq_code.txt");
    remove("run_groq.ps1");
    
    cout << "\n";
    printDivider();
}

// ─────────────────────────────────────────
//  FEATURE: CODE METRICS & STATISTICS
// ─────────────────────────────────────────
void codeMetricsMenu() {
    cout << "\n"; printDivider();
    cout << "  CODE METRICS & STATISTICS\n"; printDivider();
    
    string filename;
    cout << "  Enter file name (or full path) to analyze: ";
    getline(cin >> ws, filename);
    if (filename.size() >= 2 && filename.front() == '"' && filename.back() == '"') {
        filename = filename.substr(1, filename.size() - 2);
    }
    
    string raw;
    if (!readRaw(filename, raw)) return;

    int totalLines = 0, blankLines = 0, comments = 0;
    int loops = 0, conditionals = 0;
    
    stringstream ss(raw);
    string line;
    while(getline(ss, line)) {
        totalLines++;
        string t = trim(line);
        if (t.empty()) { blankLines++; continue; }
        
        if (t.find("//") != string::npos || t.find("/*") != string::npos || t.find("* ") != string::npos) comments++;
        
        if (t.find("for ") != string::npos || t.find("for(") != string::npos ||
            t.find("while ") != string::npos || t.find("while(") != string::npos) loops++;
            
        if (t.find("if ") != string::npos || t.find("if(") != string::npos ||
            t.find("else if") != string::npos || t.find("switch") != string::npos) conditionals++;
    }
    
    int codeLines = totalLines - blankLines - comments;
    if (codeLines < 0) codeLines = 0;

    cout << "\n  ── Metrics for " << getBaseName(filename) << " ──\n";
    cout << "  Total Lines     : " << totalLines << "\n";
    cout << "  Code Lines      : " << codeLines << "\n";
    cout << "  Blank Lines     : " << blankLines << "\n";
    cout << "  Comments        : " << comments << "\n";
    cout << "  Loops           : " << loops << "\n";
    cout << "  Conditionals    : " << conditionals << "\n";
    
    int score = 100 - (loops * 2) - (conditionals) + (comments * 2);
    if (score > 100) score = 100;
    if (score < 10) score = 10;
    
    cout << "  Readability     : " << score << "/100\n";
    cout << "\n";
    printDivider();
}

// ─────────────────────────────────────────
//  MAIN MENU
// ─────────────────────────────────────────
void showMenu() {
    while (true) {
        cout << "\n";
        printDivider('=');
        cout << "    CODE PLAGIARISM DETECTOR\n";
        cout << "    + Code Beautifier\n";
        printDivider('=');
        cout << "    1. Compare Files (up to 10)\n";
        cout << "    2. Beautify / Format Code (with Comment Remover)\n";
        cout << "    3. AI Code Analysis (via Groq API)\n";
        cout << "    4. Code Metrics & Statistics\n";
        cout << "    5. Exit\n";
        printDivider('=');
        cout << "    Enter choice (1-5): ";

        int choice;
        cin >> choice;

        if (cin.fail()) {
            cin.clear(); cin.ignore(10000, '\n');
            cout << "\n  [ERROR] Invalid input — enter 1, 2, 3, 4, or 5.\n";
            continue;
        }

        if      (choice == 1) compareFilesMenu();
        else if (choice == 2) beautifyMenu();
        else if (choice == 3) codeAnalysisMenu();
        else if (choice == 4) codeMetricsMenu();
        else if (choice == 5) { cout << "\n  Goodbye!\n\n"; break; }
        else    cout << "\n  [ERROR] Enter 1, 2, 3, 4, or 5.\n";
    }
}

int main() { showMenu(); return 0; }
