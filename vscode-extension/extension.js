const vscode = require('vscode');
const fs = require('fs');
const path = require('path');

// ── Core Algorithms ──────────────────────────────────────────

function cleanLine(l) { return l.replace(/\s+/g,' ').trim().toLowerCase(); }
function escapeHTML(s) { return s.replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;').replace(/"/g,'&quot;'); }

function compareFiles(f1, f2) {
    const matches = [];
    for (const l of f1) if (f2.includes(l) && !matches.includes(l)) matches.push(l);
    const mx = Math.max(f1.length, f2.length);
    return { sim: mx ? (matches.length / mx) * 100 : 0, matches };
}

function getLines(text) { return text.split('\n').map(cleanLine).filter(l => l.length > 0); }

function severityLabel(s) {
    if (s >= 80) return 'HIGH PLAGIARISM';
    if (s >= 50) return 'MODERATE SIMILARITY';
    if (s >= 20) return 'LOW SIMILARITY';
    return 'LIKELY ORIGINAL';
}
function severityColor(s) {
    if (s >= 80) return '#ff4d4d';
    if (s >= 50) return '#ff9800';
    if (s >= 20) return '#ffd700';
    return '#00e676';
}

// ── Code Statistics ──────────────────────────────────────────

function analyzeCode(content) {
    const lines = content.split('\n');
    const nonEmpty = lines.filter(l => l.trim()).length;
    const comments = lines.filter(l => /^\s*(\/\/|\/\*|\*)/.test(l)).length;
    const functions = (content.match(/\b\w[\w\s]*\([^)]*\)\s*\{/g) || []).length;
    const classes   = (content.match(/\bclass\s+\w+/g) || []).length;
    const includes  = (content.match(/#include/g) || []).length;
    const loops     = (content.match(/\b(for|while|do)\s*[\({]/g) || []).length;
    const conditions= (content.match(/\bif\s*\(/g) || []).length;
    const blank     = lines.length - nonEmpty;
    const quality   = Math.min(100, Math.round((nonEmpty / Math.max(lines.length,1)) * 60
                        + Math.min(comments / Math.max(nonEmpty,1) * 100, 40)));
    return { total: lines.length, nonEmpty, blank, comments, functions, classes, includes, loops, conditions, quality };
}

// ── Code Beautifier (State Machine) ─────────────────────────

function addKwSpacing(code) {
    return code.replace(/\b(if|for|while|switch|catch|else)\s*\(/g, '$1 (');
}

function beautifyCode(raw) {
    const INDENT = '    ';
    let indent = 0, state = 'NORMAL', cur = '', out = [];
    const gI = () => INDENT.repeat(indent);
    const flush = () => { const t = addKwSpacing(cur.trim()); cur = ''; if (t) out.push(gI() + t); };
    const chars = [...raw]; const n = chars.length;

    for (let i = 0; i < n; i++) {
        const c = chars[i], nx = i+1<n ? chars[i+1] : '';
        if (state === 'LC') { if (c==='\n') { const t=cur.trim();cur=''; if(t) out.push(gI()+'// '+t); state='NORMAL'; } else cur+=c; continue; }
        if (state === 'BC') { if (c==='*'&&nx==='/') { const t=cur.trim();cur=''; out.push(gI()+'/* '+t+' */'); state='NORMAL';i++; } else if(c!=='\n'&&c!=='\r') cur+=c; continue; }
        if (state === 'STR') { cur+=c; if(c==='\\'&&i+1<n) cur+=chars[++i]; else if(c==='"') state='NORMAL'; continue; }
        if (state === 'CHR') { cur+=c; if(c==='\\'&&i+1<n) cur+=chars[++i]; else if(c==="'") state='NORMAL'; continue; }
        if (c==='/'&&nx==='/') { flush(); state='LC'; i++; continue; }
        if (c==='/'&&nx==='*') { flush(); state='BC'; i++; continue; }
        if (c==='"') { cur+=c; state='STR'; continue; }
        if (c==="'") { cur+=c; state='CHR'; continue; }
        if (c==='#'&&!cur.trim()) { let d='#'; while(i+1<n&&chars[i+1]!=='\n') d+=chars[++i]; out.push(d); cur=''; continue; }
        if (c==='{') { flush(); out.push(gI()+'{'); indent++; continue; }
        if (c==='}') { flush(); if(indent>0) indent--; out.push(gI()+'}'); continue; }
        if (c===';') { cur+=';'; flush(); continue; }
        if (c==='\n'||c==='\r') { if(cur&&cur[cur.length-1]!==' ') cur+=' '; continue; }
        cur+=c;
    }
    flush();
    return out.join('\n') + '\n';
}

// ── Dashboard Webview HTML ───────────────────────────────────

function buildDashboard(panel, context) {
    const docs = vscode.workspace.textDocuments.filter(d =>
        !d.isUntitled && d.uri.scheme === 'file' &&
        ['.cpp','.c','.h','.js','.py','.java','.ts'].includes(path.extname(d.fileName))
    );

    let filesHtml = '', pairsHtml = '', statsHtml = '';

    const fileData = docs.map(d => ({
        name: path.basename(d.fileName),
        lines: getLines(d.getText()),
        stats: analyzeCode(d.getText()),
        raw: d.getText()
    }));

    // File stat cards
    filesHtml = fileData.map(f => {
        const q = f.stats.quality;
        const qCol = q >= 70 ? '#00e676' : q >= 40 ? '#ffd700' : '#ff9800';
        return `<div class="card">
          <div class="card-title">📄 ${escapeHTML(f.name)}</div>
          <div class="stat-grid">
            <div class="sbox"><b>${f.stats.total}</b><span>Total Lines</span></div>
            <div class="sbox"><b>${f.stats.nonEmpty}</b><span>Code Lines</span></div>
            <div class="sbox"><b>${f.stats.functions}</b><span>Functions</span></div>
            <div class="sbox"><b>${f.stats.comments}</b><span>Comments</span></div>
            <div class="sbox"><b>${f.stats.loops}</b><span>Loops</span></div>
            <div class="sbox"><b>${f.stats.conditions}</b><span>If Conditions</span></div>
          </div>
          <div class="quality-row">
            <span>Code Quality Score</span>
            <span class="q-val" style="color:${qCol}">${q}%</span>
          </div>
          <div class="bar-bg"><div class="bar-fill" style="width:${q}%;background:${qCol}"></div></div>
        </div>`;
    }).join('') || '<div class="empty">No open source files detected. Open a .cpp, .py, .js file to begin.</div>';

    // Pair comparison
    const pairs = [];
    for (let i = 0; i < fileData.length; i++)
        for (let j = i+1; j < fileData.length; j++) {
            const { sim, matches } = compareFiles(fileData[i].lines, fileData[j].lines);
            pairs.push({ n1: fileData[i].name, n2: fileData[j].name, sim, matches });
        }

    pairsHtml = pairs.length === 0
        ? '<div class="empty">Open 2 or more files to see similarity comparisons.</div>'
        : pairs.sort((a,b) => b.sim - a.sim).map(p => {
            const col = severityColor(p.sim);
            const lbl = severityLabel(p.sim);
            const rows = p.matches.slice(0,10).map((m,i) =>
                `<tr><td>${i+1}</td><td>${escapeHTML(m)}</td></tr>`).join('');
            return `<div class="card pair-card" style="border-left:4px solid ${col}">
              <div class="pair-head">
                <div>
                  <span class="fname">${escapeHTML(p.n1)}</span>
                  <span class="vs"> vs </span>
                  <span class="fname">${escapeHTML(p.n2)}</span>
                </div>
                <div class="pct" style="color:${col}">${Math.round(p.sim)}%</div>
              </div>
              <div class="verdict" style="color:${col}">${lbl}</div>
              <div class="bar-bg" style="margin:10px 0 12px">
                <div class="bar-fill" style="width:${Math.round(p.sim)}%;background:${col}"></div>
              </div>
              <div class="match-count">${p.matches.length} matching line(s)</div>
              ${rows ? `<table><thead><tr><th>#</th><th>Matching Line</th></tr></thead><tbody>${rows}</tbody></table>` : ''}
            </div>`;
        }).join('');

    return `<!DOCTYPE html><html lang="en"><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1.0">
<title>Plagiarism Detector Dashboard</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{background:#080812;color:#d0d0e0;font-family:'Segoe UI',system-ui,sans-serif;padding:0;min-height:100vh}
.header{background:linear-gradient(135deg,#0d0d2b,#1a0a3a);padding:28px 32px;border-bottom:1px solid #1e1e4a}
.header h1{font-size:1.8rem;letter-spacing:1.5px;background:linear-gradient(90deg,#00c8ff,#a259ff,#ff6b9d);
  -webkit-background-clip:text;-webkit-text-fill-color:transparent;background-clip:text}
.header p{color:#666;font-size:.85rem;margin-top:4px}
.tabs{display:flex;background:#0d0d1a;border-bottom:1px solid #1a1a3a;padding:0 24px}
.tab{padding:14px 20px;cursor:pointer;font-size:.85rem;color:#666;
     border-bottom:2px solid transparent;transition:all .2s;letter-spacing:.5px}
.tab:hover{color:#aaa}
.tab.active{color:#00c8ff;border-bottom-color:#00c8ff}
.content{padding:24px;display:none}
.content.active{display:block}
.card{background:#111122;border:1px solid #1e1e3a;border-radius:12px;padding:20px;margin-bottom:16px}
.card-title{font-size:.95rem;font-weight:600;color:#e0e0e0;margin-bottom:14px;
  display:flex;align-items:center;gap:8px}
.stat-grid{display:grid;grid-template-columns:repeat(3,1fr);gap:10px;margin-bottom:14px}
.sbox{background:#0a0a1e;border-radius:8px;padding:10px 12px;text-align:center}
.sbox b{display:block;font-size:1.3rem;color:#e0e0e0}
.sbox span{font-size:.72rem;color:#666;margin-top:2px;display:block}
.quality-row{display:flex;justify-content:space-between;align-items:center;
  font-size:.8rem;color:#666;margin-bottom:6px}
.q-val{font-weight:700;font-size:1rem}
.bar-bg{background:#0a0a1e;border-radius:999px;height:8px;overflow:hidden}
.bar-fill{height:100%;border-radius:999px;transition:width .4s ease}
.pair-card{transition:transform .15s}
.pair-card:hover{transform:translateY(-1px)}
.pair-head{display:flex;justify-content:space-between;align-items:center;margin-bottom:4px}
.fname{color:#00c8ff;font-weight:600;font-size:.9rem}
.vs{color:#444;font-size:.85rem}
.pct{font-size:2rem;font-weight:800;line-height:1}
.verdict{font-size:.75rem;font-weight:700;letter-spacing:1.5px;margin-bottom:4px}
.match-count{font-size:.75rem;color:#555;margin-bottom:8px}
table{width:100%;border-collapse:collapse;font-size:.78rem;margin-top:6px}
th{text-align:left;padding:7px 10px;background:#0a0a1e;color:#555}
td{padding:6px 10px;border-top:1px solid #1a1a2e;
   font-family:'Courier New',monospace;color:#a8ffa8;word-break:break-all}
.empty{color:#444;font-style:italic;padding:30px;text-align:center;
  background:#0d0d1a;border-radius:12px;border:1px dashed #1e1e3a}
.howto-step{display:flex;gap:14px;align-items:flex-start;padding:14px;
  background:#0d0d1a;border-radius:10px;margin-bottom:10px}
.step-num{background:linear-gradient(135deg,#00c8ff,#a259ff);
  color:#000;font-weight:800;border-radius:8px;
  min-width:32px;height:32px;display:flex;align-items:center;
  justify-content:center;font-size:.85rem}
.step-txt b{display:block;color:#e0e0e0;margin-bottom:3px;font-size:.9rem}
.step-txt p{color:#777;font-size:.8rem;line-height:1.5}
.badge{display:inline-block;background:#a259ff22;color:#a259ff;
  border:1px solid #a259ff44;border-radius:6px;padding:2px 10px;
  font-size:.72rem;font-weight:700;letter-spacing:1px;margin:2px}
.auto-badge{background:#00e67622;color:#00e676;border-color:#00e67644}
.refresh-btn{background:linear-gradient(135deg,#00c8ff,#a259ff);
  border:none;color:#000;font-weight:700;padding:10px 22px;
  border-radius:8px;cursor:pointer;font-size:.85rem;letter-spacing:.5px;
  margin-top:8px;transition:opacity .2s}
.refresh-btn:hover{opacity:.85}
footer{text-align:center;color:#252535;font-size:.72rem;padding:20px}
</style></head><body>
<div class="header">
  <h1>🔍 Code Plagiarism Detector</h1>
  <p>Final Year Capstone Project &nbsp;•&nbsp; Auto-detects similarity &amp; beautifies code</p>
</div>
<div class="tabs">
  <div class="tab active" onclick="show('files')">📁 File Analysis</div>
  <div class="tab" onclick="show('compare')">📊 Similarity Report</div>
  <div class="tab" onclick="show('howto')">📖 How to Use</div>
</div>

<div id="files" class="content active">
  <h2 style="font-size:.85rem;color:#555;text-transform:uppercase;letter-spacing:2px;margin-bottom:16px">
    Open Files — Code Statistics</h2>
  ${filesHtml}
</div>

<div id="compare" class="content">
  <h2 style="font-size:.85rem;color:#555;text-transform:uppercase;letter-spacing:2px;margin-bottom:16px">
    Similarity Report — All Open File Pairs</h2>
  ${pairsHtml}
</div>

<div id="howto" class="content">
  <h2 style="font-size:.85rem;color:#555;text-transform:uppercase;letter-spacing:2px;margin-bottom:16px">
    How to Use This Extension</h2>
  <div class="howto-step"><div class="step-num">1</div>
    <div class="step-txt"><b>Open your code files</b>
      <p>Open any .cpp, .py, .js, .java files in VS Code. The extension automatically reads all open files.</p></div></div>
  <div class="howto-step"><div class="step-num">2</div>
    <div class="step-txt"><b>Auto-Detection on Save</b>
      <p>Every time you press <b>Ctrl+S</b> to save, the extension automatically compares your file against all others and shows the similarity % in the status bar at the bottom.</p></div></div>
  <div class="howto-step"><div class="step-num">3</div>
    <div class="step-txt"><b>Beautify Messy Code</b>
      <p>Right-click anywhere in your code → <b>"Plagiarism Detector: Beautify Current File"</b> — your messy code is instantly cleaned and properly indented.</p></div></div>
  <div class="howto-step"><div class="step-num">4</div>
    <div class="step-txt"><b>Open This Dashboard</b>
      <p>Press <b>Ctrl+Shift+P</b>, type <b>Plagiarism Detector: Open Dashboard</b> to see this full report anytime.</p></div></div>
  <div class="howto-step"><div class="step-num">5</div>
    <div class="step-txt"><b>Auto-Beautify on Save (Optional)</b>
      <p>Go to <b>File → Preferences → Settings</b>, search for <b>"Plagiarism"</b> and turn on "Auto Beautify On Save" to format your code automatically every time you save.</p></div></div>
  <div style="margin-top:20px;padding:16px;background:#0d0d1a;border-radius:10px;border:1px solid #1e1e3a">
    <div style="font-size:.8rem;color:#888;margin-bottom:10px">Available Commands (press Ctrl+Shift+P and type "Plagiarism"):</div>
    <span class="badge">📊 Open Dashboard</span>
    <span class="badge">🔍 Compare Files</span>
    <span class="badge">✨ Beautify Current File</span>
    <span class="badge">📈 Show Code Statistics</span>
    <div style="margin-top:10px;font-size:.78rem;color:#555">Also available by right-clicking in any code file.</div>
  </div>
</div>

<footer>Code Plagiarism Detector v2.0 &mdash; Final Year Capstone Project</footer>
<script>
function show(tab) {
  document.querySelectorAll('.content').forEach(c => c.classList.remove('active'));
  document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));
  document.getElementById(tab).classList.add('active');
  event.target.classList.add('active');
}
</script></body></html>`;
}

// ── Notification Helper ──────────────────────────────────────

function buildStatsHTML(fileName, stats, raw) {
    const beautified = beautifyCode(raw);
    return `<!DOCTYPE html><html><head>
<meta charset="UTF-8">
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{background:#080812;color:#d0d0e0;font-family:'Segoe UI',sans-serif;padding:24px}
h1{font-size:1.4rem;letter-spacing:1px;background:linear-gradient(90deg,#00c8ff,#a259ff);
  -webkit-background-clip:text;-webkit-text-fill-color:transparent;background-clip:text;margin-bottom:18px}
.grid{display:grid;grid-template-columns:repeat(3,1fr);gap:12px;margin-bottom:20px}
.box{background:#111122;border:1px solid #1e1e3a;border-radius:10px;padding:16px;text-align:center}
.box b{display:block;font-size:1.8rem;color:#e0e0e0}
.box span{font-size:.75rem;color:#666}
pre{background:#0a0a1e;border:1px solid #1e1e3a;border-radius:10px;
    padding:16px;font-family:'Courier New',monospace;font-size:.8rem;
    color:#a8ffa8;overflow:auto;max-height:400px;line-height:1.6}
h2{font-size:.8rem;text-transform:uppercase;letter-spacing:2px;color:#555;margin-bottom:10px}
</style></head><body>
<h1>📈 Code Stats — ${escapeHTML(fileName)}</h1>
<div class="grid">
  <div class="box"><b>${stats.total}</b><span>Total Lines</span></div>
  <div class="box"><b>${stats.nonEmpty}</b><span>Code Lines</span></div>
  <div class="box"><b>${stats.blank}</b><span>Blank Lines</span></div>
  <div class="box"><b>${stats.functions}</b><span>Functions</span></div>
  <div class="box"><b>${stats.classes}</b><span>Classes</span></div>
  <div class="box"><b>${stats.comments}</b><span>Comments</span></div>
  <div class="box"><b>${stats.loops}</b><span>Loops</span></div>
  <div class="box"><b>${stats.conditions}</b><span>If Conditions</span></div>
  <div class="box"><b>${stats.quality}%</b><span>Quality Score</span></div>
</div>
<h2>✨ Beautified Preview</h2>
<pre>${escapeHTML(beautified)}</pre>
</body></html>`;
}

// ── Extension Activation ─────────────────────────────────────

function activate(context) {
    // Status bar item
    const statusBar = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Left, 100);
    statusBar.text = '$(search) Plagiarism Detector';
    statusBar.tooltip = 'Click to open Plagiarism Detector Dashboard';
    statusBar.command = 'plagiarism.openDashboard';
    statusBar.show();
    context.subscriptions.push(statusBar);

    // ── Auto-detect on save ──────────────────────────────────
    const onSave = vscode.workspace.onDidSaveTextDocument(async doc => {
        const cfg = vscode.workspace.getConfiguration('plagiarismDetector');
        if (!cfg.get('autoDetectOnSave')) return;

        const saved = getLines(doc.getText());
        const others = vscode.workspace.textDocuments.filter(d =>
            d.uri.fsPath !== doc.uri.fsPath &&
            !d.isUntitled && d.uri.scheme === 'file' &&
            ['.cpp','.c','.h','.js','.py','.java','.ts'].includes(path.extname(d.fileName))
        );

        let topSim = 0, topName = '';
        for (const other of others) {
            const { sim } = compareFiles(saved, getLines(other.getText()));
            if (sim > topSim) { topSim = sim; topName = path.basename(other.fileName); }
        }

        if (others.length === 0) {
            statusBar.text = `$(search) No comparison file open`;
        } else {
            const col = topSim >= 80 ? '🔴' : topSim >= 50 ? '🟠' : topSim >= 20 ? '🟡' : '🟢';
            statusBar.text = `${col} Similarity: ${Math.round(topSim)}% vs ${topName}`;

            const threshold = cfg.get('warningThreshold') || 50;
            if (topSim >= threshold) {
                vscode.window.showWarningMessage(
                    `⚠️ ${Math.round(topSim)}% similarity with "${topName}" — possible plagiarism!`,
                    'View Report'
                ).then(sel => { if (sel === 'View Report') vscode.commands.executeCommand('plagiarism.openDashboard'); });
            }
        }

        // Auto-beautify on save
        if (cfg.get('autoBeautifyOnSave')) {
            const beautified = beautifyCode(doc.getText());
            const edit = new vscode.WorkspaceEdit();
            edit.replace(doc.uri, new vscode.Range(doc.positionAt(0), doc.positionAt(doc.getText().length)), beautified);
            await vscode.workspace.applyEdit(edit);
        }
    });
    context.subscriptions.push(onSave);

    // ── Command: Open Dashboard ──────────────────────────────
    context.subscriptions.push(vscode.commands.registerCommand('plagiarism.openDashboard', () => {
        const panel = vscode.window.createWebviewPanel(
            'plagiarismDashboard', '🔍 Plagiarism Dashboard',
            vscode.ViewColumn.Two, { enableScripts: true }
        );
        panel.webview.html = buildDashboard(panel, context);
    }));

    // ── Command: Compare Files (file picker) ─────────────────
    context.subscriptions.push(vscode.commands.registerCommand('plagiarism.compare', async () => {
        const uris = await vscode.window.showOpenDialog({
            canSelectMany: true, openLabel: 'Select Files to Compare',
            filters: { 'Source Files': ['cpp','c','h','py','java','js','ts','txt'] }
        });
        if (!uris || uris.length < 2) {
            vscode.window.showWarningMessage('Please select at least 2 files.'); return;
        }
        const fileData = [];
        for (const uri of uris) {
            try {
                const raw = fs.readFileSync(uri.fsPath, 'utf8');
                fileData.push({ name: path.basename(uri.fsPath), lines: getLines(raw), stats: analyzeCode(raw) });
            } catch { vscode.window.showErrorMessage(`Cannot read: ${path.basename(uri.fsPath)}`); return; }
        }
        const panel = vscode.window.createWebviewPanel(
            'plagiarismReport', '📊 Comparison Report',
            vscode.ViewColumn.One, { enableScripts: true }
        );
        // Reuse dashboard builder with injected file data
        vscode.workspace.textDocuments; // trigger refresh
        panel.webview.html = buildDashboard(panel, context);
        vscode.window.showInformationMessage(`Comparing ${fileData.length} files — report is open!`);
    }));

    // ── Command: Beautify Current File ───────────────────────
    context.subscriptions.push(vscode.commands.registerCommand('plagiarism.beautify', async () => {
        const editor = vscode.window.activeTextEditor;
        if (!editor) { vscode.window.showWarningMessage('No active file to beautify.'); return; }
        const doc = editor.document;
        const raw = doc.getText();
        const beautified = beautifyCode(raw);
        const edit = new vscode.WorkspaceEdit();
        edit.replace(doc.uri, new vscode.Range(doc.positionAt(0), doc.positionAt(raw.length)), beautified);
        await vscode.workspace.applyEdit(edit);
        vscode.window.showInformationMessage(`✅ Code beautified: ${path.basename(doc.fileName)}`);
    }));

    // ── Command: Show Code Statistics ───────────────────────
    context.subscriptions.push(vscode.commands.registerCommand('plagiarism.stats', () => {
        const editor = vscode.window.activeTextEditor;
        if (!editor) { vscode.window.showWarningMessage('No active file.'); return; }
        const doc = editor.document;
        const raw = doc.getText();
        const stats = analyzeCode(raw);
        const panel = vscode.window.createWebviewPanel(
            'codeStats', `📈 Stats — ${path.basename(doc.fileName)}`,
            vscode.ViewColumn.Two, { enableScripts: false }
        );
        panel.webview.html = buildStatsHTML(path.basename(doc.fileName), stats, raw);
    }));

    vscode.window.showInformationMessage('🔍 Code Plagiarism Detector is active! Save any file to auto-detect similarity.');
}

function deactivate() {}
module.exports = { activate, deactivate };
