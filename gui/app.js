// Compiler GUI Client-side Logic

document.addEventListener('DOMContentLoaded', () => {
    // UI Elements
    const codeTextarea = document.getElementById('code-textarea');
    const lineNumbers = document.getElementById('line-numbers');
    const compileBtn = document.getElementById('compile-btn');
    const statusBadge = document.getElementById('status-badge');
    const fileUpload = document.getElementById('file-upload');
    const currentFilename = document.getElementById('current-filename');
    const consolePanel = document.getElementById('console-panel');
    const consoleBody = document.getElementById('console-body');
    const consoleClose = document.getElementById('console-close');
    const rdTreeContent = document.getElementById('rd-tree-content');
    const ll1Content = document.getElementById('ll1-content');
    const lr1Content = document.getElementById('lr1-content');
    let currentTokens = [];
    let rawTreeText = "";
    let currentTreeMode = "CST";

    // Start with blank editor
    codeTextarea.value = '';
    updateLineNumbers();

    // Editor Synced Line Numbers
    codeTextarea.addEventListener('input', () => {
        updateLineNumbers();
        clearErrorHighlights();
    });

    codeTextarea.addEventListener('scroll', () => {
        lineNumbers.scrollTop = codeTextarea.scrollTop;
    });

    function updateLineNumbers() {
        const text = codeTextarea.value;
        const lines = text.split('\n');
        const count = lines.length;
        
        let html = '';
        for (let i = 1; i <= count; i++) {
            html += `<div id="line-num-${i}">${i}</div>`;
        }
        lineNumbers.innerHTML = html;
        lineNumbers.scrollTop = codeTextarea.scrollTop;
    }

    function highlightErrorLine(lineNum) {
        clearErrorHighlights();
        const lineEl = document.getElementById(`line-num-${lineNum}`);
        if (lineEl) {
            lineEl.classList.add('highlight-error-num');
            // Scroll to the line
            const textareaHeight = codeTextarea.clientHeight;
            const lineHeight = 24; // approximate height per line in px
            codeTextarea.scrollTop = (lineNum - 1) * lineHeight - (textareaHeight / 3);
            lineNumbers.scrollTop = codeTextarea.scrollTop;
        }
    }

    function clearErrorHighlights() {
        const lines = lineNumbers.children;
        for (let line of lines) {
            line.classList.remove('highlight-error-num');
        }
    }

    // Tabs Manager
    const tabButtons = document.querySelectorAll('.tab-btn');
    const tabPanes = document.querySelectorAll('.tab-pane');

    tabButtons.forEach(btn => {
        btn.addEventListener('click', () => {
            const targetTab = btn.getAttribute('data-tab');
            
            tabButtons.forEach(b => b.classList.remove('active'));
            tabPanes.forEach(p => p.classList.remove('active'));
            
            btn.classList.add('active');
            document.getElementById(`tab-${targetTab}`).classList.add('active');
        });
    });

    // File Upload
    fileUpload.addEventListener('change', (e) => {
        const file = e.target.files[0];
        if (!file) return;

        currentFilename.textContent = file.name;
        const reader = new FileReader();
        reader.onload = (event) => {
            codeTextarea.value = event.target.result;
            updateLineNumbers();
            clearErrorHighlights();
            resetDashboard();
        };
        reader.readAsText(file);
        
        // Reset file upload value so the same file can be uploaded consecutively
        fileUpload.value = '';
    });

    // Console Panel Close
    consoleClose.addEventListener('click', () => {
        consolePanel.style.display = 'none';
    });

    // Reset Dashboard to empty state
    function resetDashboard() {
        statusBadge.textContent = 'Idle';
        statusBadge.className = 'status-badge';
        consolePanel.style.display = 'none';
        
        // Reset Tabs to loading/empty state
        document.querySelector('#tab-lexer tbody').innerHTML = '<tr><td colspan="4" class="empty-state">No tokens scanned. Click compile.</td></tr>';
        rdTreeContent.innerHTML = '<div class="empty-state">No parse tree generated. Click compile.</div>';
        document.querySelector('#tab-sym-table tbody').innerHTML = '<tr><td colspan="5" class="empty-state">No symbols loaded. Click compile.</td></tr>';
        ll1Content.innerHTML = '<div class="empty-state">No LL(1) derivations yet. Click compile.</div>';
        lr1Content.innerHTML = '<div class="empty-state">No LR(1) actions logged yet. Click compile.</div>';
        
        // Reset VM
        document.getElementById('vm-empty-state').style.display = 'block';
        document.getElementById('vm-layout').style.display = 'none';
        document.getElementById('run-vm-btn').style.display = 'none';
        document.getElementById('reset-vm-btn').style.display = 'none';
    }

    // Compile Trigger
    compileBtn.addEventListener('click', async () => {
        if (compileBtn.classList.contains('loading')) return;

        compileBtn.classList.add('loading');
        statusBadge.textContent = 'Compiling...';
        statusBadge.className = 'status-badge';
        clearErrorHighlights();
        resetDashboard();

        const code = codeTextarea.value;

        try {
            const compileUrl = window.location.protocol === 'file:' 
                ? 'http://localhost:8000/compile' 
                : '/compile';

            const response = await fetch(compileUrl, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ code: code })
            });

            if (!response.ok) {
                throw new Error('HTTP request failed');
            }

            const data = await response.json();
            
            // Clean up outputs (remove ANSI color escape sequences)
            const cleanStdout = stripAnsi(data.stdout || '');
            const cleanStderr = stripAnsi(data.stderr || '');
            
            parseCompilationResults(data.status, cleanStdout, cleanStderr);

        } catch (err) {
            console.error(err);
            statusBadge.textContent = 'Connection Error';
            statusBadge.className = 'status-badge compile-error';
            showSystemError("Failed to connect to the local compiler server. Make sure `server.py` is running.");
        } finally {
            compileBtn.classList.remove('loading');
        }
    });

    // Helper to strip ANSI codes
    function stripAnsi(str) {
        return str.replace(/[\u001b\u009b][[()#;?]*(?:[0-9]{1,4}(?:;[0-9]{0,4})*)?[0-9A-ORZcf-nqry=><]/g, '');
    }

    // Parse Outputs and populate tabs
    function parseCompilationResults(status, stdout, stderr) {
        // --- 1. Detect Parsing Errors ---
        const errorPattern = /(Semantic Error|Lexical Error|Syntax Error|LL\(1\) Error|LR\(1\) Error|Error) at line (\d+), col (\d+): (.*)/gi;
        let errorsMap = new Map();
        let match;
        
        // Search both stdout and stderr
        const fullOutput = stdout + "\n" + stderr;
        while ((match = errorPattern.exec(fullOutput)) !== null) {
            const errType = match[1].trim();
            const line = parseInt(match[2]);
            const col = parseInt(match[3]);
            const msg = match[4].trim();
            const key = `${line}:${col}`;
            
            if (!errorsMap.has(key)) {
                errorsMap.set(key, {
                    type: errType,
                    line: line,
                    col: col,
                    messages: [msg]
                });
            } else {
                const current = errorsMap.get(key);
                if (!current.messages.includes(msg)) {
                    current.messages.push(msg);
                }
                const currentTypeLower = current.type.toLowerCase();
                const newTypeLower = errType.toLowerCase();
                if ((newTypeLower.includes("lexical") || newTypeLower.includes("semantic") || newTypeLower.includes("syntax")) && 
                    (!currentTypeLower.includes("lexical") && !currentTypeLower.includes("semantic") && !currentTypeLower.includes("syntax"))) {
                    current.type = errType;
                }
            }
        }

        let errors = Array.from(errorsMap.values());

        if (errors.length > 0) {
            // Display errors
            statusBadge.textContent = 'Failed';
            statusBadge.className = 'status-badge compile-error';
            renderErrors(errors);
            // Highlight the first error line
            highlightErrorLine(errors[0].line);
            
            // Lock VM
            document.getElementById('vm-empty-state').style.display = 'block';
            document.getElementById('vm-layout').style.display = 'none';
            document.getElementById('run-vm-btn').style.display = 'none';
            document.getElementById('reset-vm-btn').style.display = 'none';
        } else {
            statusBadge.textContent = 'Success';
            statusBadge.className = 'status-badge compile-success';
            consolePanel.style.display = 'none';
            
            // Unlock VM
            document.getElementById('vm-empty-state').style.display = 'none';
            document.getElementById('run-vm-btn').style.display = 'inline-flex';
        }

        // --- 2. Extract Phase 1: Lexer Tokens ---
        const lexerSection = extractSection(stdout, 'PHASE 1: LEXICAL ANALYSIS (LEXER)', 'PHASE 2: RECURSIVE DESCENT PARSE TREE');
        if (lexerSection) {
            parseLexerTokens(lexerSection);
        }

        // --- 3. Extract Phase 2: Recursive Descent Parse Tree ---
        const rdTreeSection = extractSection(stdout, 'PHASE 2: RECURSIVE DESCENT PARSE TREE', 'PHASE 3: SYMBOL TABLE DUMP');
        rawTreeText = rdTreeSection || "";
        updateTreeDisplay();

        // --- 4. Extract Phase 3: Symbol Table ---
        const symTableSection = extractSection(stdout, 'PHASE 3: SYMBOL TABLE DUMP', 'PHASE 4: LL(1) PREDICTIVE PARSER STEPS');
        if (symTableSection) {
            parseSymbolTable(symTableSection);
        }

        // --- 5. Extract Phase 4: LL(1) Steps ---
        const ll1Section = extractSection(stdout, 'PHASE 4: LL(1) PREDICTIVE PARSER STEPS', 'PHASE 5: CANONICAL LR(1) PARSER ACTIONS');
        if (ll1Section) {
            ll1Content.innerHTML = formatDerivations(ll1Section);
        }

        // --- 6. Extract Phase 5: LR(1) Actions ---
        const lr1Section = extractSection(stdout, 'PHASE 5: CANONICAL LR(1) PARSER ACTIONS', '');
        if (lr1Section) {
            lr1Content.innerHTML = formatDerivations(lr1Section);
        }
    }

    // Helper to extract a section between two string markers
    function extractSection(text, startMarker, endMarker) {
        const startIdx = text.indexOf(startMarker);
        if (startIdx === -1) return null;
        
        let subText = text.substring(startIdx + startMarker.length);
        if (endMarker) {
            const endIdx = subText.indexOf(endMarker);
            if (endIdx !== -1) {
                subText = subText.substring(0, endIdx);
            }
        }
        
        // Strip the boundary line designs (===...)
        return subText.replace(/^[\s\S]*?===+\r?\n/, '').trim();
    }

    // Parse Lexer Row Entries
    function parseLexerTokens(text) {
        const rows = text.split('\n');
        let tbodyHtml = '';
        let tokenCount = 0;
        currentTokens = [];

        for (let row of rows) {
            const regex = /\|\s*(.*?)\s*\|\s*(.*?)\s*\|\s*(\d+)\s*\|\s*(\d+)\s*\|/;
            const match = row.match(regex);
            
            if (match) {
                const type = match[1];
                const lexeme = match[2];
                const line = match[3];
                const col = match[4];
                
                currentTokens.push({
                    type: type,
                    lexeme: lexeme,
                    line: parseInt(line),
                    col: parseInt(col)
                });
                
                // Color formatting depending on token kind
                let styleClass = '';
                if (type.startsWith('KW_')) styleClass = 'style="color: #a855f7; font-weight: bold;"'; // keyword purple
                else if (type === 'IDENTIFIER') styleClass = 'style="color: #06b6d4;"'; // cyan
                else if (type === 'NUMBER' || type === 'CHAR_CONST') styleClass = 'style="color: #f59e0b;"'; // yellow
                else if (type === 'ERROR') styleClass = 'style="color: #ef4444; font-weight: bold;"'; // red
                
                tbodyHtml += `<tr>
                    <td><span ${styleClass}>${type}</span></td>
                    <td><code>${escapeHtml(lexeme)}</code></td>
                    <td>${line}</td>
                    <td>${col}</td>
                </tr>`;
                tokenCount++;
            }
        }

        if (tbodyHtml) {
            document.querySelector('#tab-lexer tbody').innerHTML = tbodyHtml;
        } else {
            document.querySelector('#tab-lexer tbody').innerHTML = '<tr><td colspan="4" class="empty-state">No tokens found.</td></tr>';
        }
    }

    // Format Tree Hierarchy
    function formatTree(text) {
        const lines = text.split('\n');
        let formatted = '';
        for (let line of lines) {
            if (line.includes('[SUCCESS]') || line.includes('[FAILED]') || line.trim() === '' || line.startsWith('---')) {
                continue;
            }
            // Highlight node name and token value
            const formattedLine = line
                .replace(/├── ([A-Za-z0-9_]+)/g, '├── <span style="color: #f59e0b; font-weight: bold;">$1</span>')
                .replace(/│  /g, '<span style="color: #4b5563;">│  </span>')
                .replace(/\(token: "([^"]+)"\)/g, '(<span style="color: #06b6d4; font-style: italic;">"$1"</span>)');
                
            formatted += formattedLine + '\n';
        }
        return formatted ? `<pre style="line-height: 1.7;">${formatted}</pre>` : '<div class="empty-state">Empty Tree.</div>';
    }

    // Parse Symbol Table Row Entries
    function parseSymbolTable(text) {
        const rows = text.split('\n');
        let tbodyHtml = '';

        for (let row of rows) {
            const regex = /\|\s*(\d+)\s*\|\s*(.*?)\s*\|\s*(.*?)\s*\|\s*(.*?)\s*\|\s*(\d+)\s*\|/;
            const match = row.match(regex);
            
            if (match) {
                const scope = match[1];
                const name = match[2];
                const kind = match[3];
                const type = match[4];
                const line = match[5];
                
                let kindColor = '';
                if (kind === 'METHOD') kindColor = 'style="color: #a855f7;"';
                else if (kind === 'CLASS') kindColor = 'style="color: #3b82f6; font-weight: bold;"';
                else if (kind === 'CONST') kindColor = 'style="color: #f59e0b;"';
                
                tbodyHtml += `<tr>
                    <td><span class="badge">${scope}</span></td>
                    <td><strong>${name}</strong></td>
                    <td><span ${kindColor}>${kind}</span></td>
                    <td><code>${escapeHtml(type)}</code></td>
                    <td>${line}</td>
                </tr>`;
            }
        }

        if (tbodyHtml) {
            document.querySelector('#tab-sym-table tbody').innerHTML = tbodyHtml;
        } else {
            document.querySelector('#tab-sym-table tbody').innerHTML = '<tr><td colspan="5" class="empty-state">No user symbols found.</td></tr>';
        }
    }

    // Format terminal style parsing derivations
    function formatDerivations(text) {
        if (text.includes('[LL_STEP]')) {
            return formatStepsTable(text, '[LL_STEP]');
        }
        if (text.includes('[LR_STEP]')) {
            return formatStepsTable(text, '[LR_STEP]');
        }
        
        const lines = text.split('\n');
        let html = '';
        for (let line of lines) {
            if (line.includes('[SUCCESS]') || line.includes('[FAILED]') || line.trim() === '' || line.startsWith('---')) {
                continue;
            }
            
            let styledLine = escapeHtml(line);
            
            // Highlight transitions and rules
            if (line.startsWith('Applied Rule:')) {
                styledLine = styledLine
                    .replace('Applied Rule:', '<span style="color: #64748b;">Applied Rule:</span>')
                    .replace(/&gt; (.*)/, '-&gt; <span style="color: #10b981;">$1</span>');
            } else if (line.startsWith('Action:')) {
                styledLine = styledLine
                    .replace('Action:', '<span style="color: #64748b;">Action:</span>')
                    .replace('SHIFT', '<span style="color: #3b82f6; font-weight: bold;">SHIFT</span>')
                    .replace('REDUCE', '<span style="color: #a855f7; font-weight: bold;">REDUCE</span>')
                    .replace('ACCEPT', '<span style="color: #10b981; font-weight: bold;">ACCEPT</span>');
            } else if (line.trim().startsWith('-&gt; GOTO') || line.trim().startsWith('-> GOTO')) {
                styledLine = `<span style="color: #06b6d4;">${styledLine}</span>`;
            }
            
            html += `<div>${styledLine}</div>`;
        }
        return html ? html : '<div class="empty-state">Empty Log.</div>';
    }

    function formatStepsTable(text, prefix) {
        const lines = text.split('\n');
        let rowsHtml = '';
        
        for (let line of lines) {
            if (!line.includes(prefix)) {
                continue;
            }
            
            const parts = line.split('|');
            if (parts.length < 3) continue;
            
            let stackPart = parts[0].replace(prefix, '').replace('Stack:', '').trim();
            let inputPart = parts[1].replace('Input:', '').trim();
            let actionPart = parts[2].replace('Action:', '').trim();
            
            // Format stack elements as bubbles
            const stackElems = stackPart.split(/\s+/).filter(x => x !== '');
            let stackHtml = '';
            for (let el of stackElems) {
                stackHtml += `<span class="step-badge stack-badge">${escapeHtml(el)}</span>`;
            }
            
            // Format input elements as bubbles
            const inputElems = inputPart.split(/\s+/).filter(x => x !== '');
            let inputHtml = '';
            for (let el of inputElems) {
                inputHtml += `<span class="step-badge input-badge">${escapeHtml(el)}</span>`;
            }
            
            // Highlight action text
            let actionHtml = escapeHtml(actionPart);
            if (actionHtml.startsWith('Shift')) {
                actionHtml = actionHtml.replace(/Shift\s+(.*)/, '<strong>Shift</strong> <span class="action-state">$1</span>');
            } else if (actionHtml.startsWith('Reduce')) {
                actionHtml = actionHtml
                    .replace(/Reduce\s+\((\d+)\)/, '<strong>Reduce</strong> (<span class="rule-index">$1</span>)')
                    .replace(/Pop\s+(\d+)\s+states?/, 'Pop <span class="pop-states">$1</span> state$2')
                    .replace(/Push\s+GOTO\(([^,]+),\s*([^)]+)\)\s*=\s*([0-9\-]+)/, 'Push <strong>GOTO</strong>($1, <span class="goto-sym">$2</span>) = <span class="goto-state">$3</span>');
            } else if (actionHtml.toLowerCase() === 'accept') {
                actionHtml = '<span class="action-accept">Accept</span>';
            } else if (actionHtml.startsWith('Match token')) {
                actionHtml = actionHtml.replace(/Match token\s+'(.*)'/, '<strong>Match</strong> \'<span class="match-token">$1</span>\'');
            } else if (actionHtml.startsWith('Applied Rule')) {
                actionHtml = actionHtml.replace(/Applied Rule\s+(.*)/, 'Applied Rule <span class="applied-rule">$1</span>');
            }
            
            rowsHtml += `<tr>
                <td><div class="step-badge-container">${stackHtml}</div></td>
                <td><div class="step-badge-container">${inputHtml}</div></td>
                <td><div class="action-container">${actionHtml}</div></td>
            </tr>`;
        }
        
        if (!rowsHtml) {
            return '<div class="empty-state">No steps recorded.</div>';
        }
        
        return `<div class="table-responsive">
            <table class="derivations-table">
                <thead>
                    <tr>
                        <th style="width: 35%;">Stack</th>
                        <th style="width: 35%;">Input Buffer</th>
                        <th style="width: 30%;">Action</th>
                    </tr>
                </thead>
                <tbody>
                    ${rowsHtml}
                </tbody>
            </table>
        </div>`;
    }

    // Render parsed errors list
    // Render parsed errors list
    function renderErrors(errorsList) {
        let html = '';
        for (let err of errorsList) {
            let errorType = "Syntactic Error"; // default
            const rawType = err.type.toLowerCase();
            
            let isLexical = rawType.includes("lexical");
            let isSemantic = rawType.includes("semantic");
            
            for (let msg of err.messages) {
                const msgLower = msg.toLowerCase();
                if (msgLower.includes("lexical") || msgLower.includes("unknown char") || msgLower.includes("malformed char")) {
                    isLexical = true;
                }
                if (msgLower.includes("semantic") || msgLower.includes("undeclared") || msgLower.includes("redeclaration") || msgLower.includes("type mismatch")) {
                    isSemantic = true;
                }
            }
            
            if (isLexical) {
                errorType = "Lexical Error";
            } else if (isSemantic) {
                errorType = "Semantic Error";
            }
            
            let msgsHtml = '';
            let guidesHtml = '';
            let processedGuides = new Set();
            
            for (let msg of err.messages) {
                msgsHtml += `<div class="error-message" style="margin-left: 10px; margin-bottom: 4px;">• ${escapeHtml(msg)}</div>`;
                const guide = generateErrorGuide(msg);
                if (!processedGuides.has(guide)) {
                    processedGuides.add(guide);
                    guidesHtml += `<div class="error-guide" style="margin-top: 4px; margin-bottom: 8px;">💡 <strong>Compilation Guide:</strong> ${guide}</div>`;
                }
            }
            
            html += `<div class="error-item" style="border-left: 4px solid #ef4444; padding-left: 12px; margin-bottom: 16px;">
                <div class="error-meta" style="font-weight: bold; font-size: 0.95rem; margin-bottom: 6px; color: #f87171;">${errorType} on Line ${err.line}, Column ${err.col}</div>
                ${msgsHtml}
                ${guidesHtml}
            </div>`;
        }
        consoleBody.innerHTML = html;
        consolePanel.style.display = 'block';
    }

    // Show custom system errors
    function showSystemError(message) {
        consoleBody.innerHTML = `<div class="error-item">
            <div class="error-meta">System Exception</div>
            <div class="error-message">${escapeHtml(message)}</div>
        </div>`;
        consolePanel.style.display = 'block';
    }

    // AST / CST Toggle Controls
    const cstBtn = document.getElementById('tree-cst-btn');
    const astBtn = document.getElementById('tree-ast-btn');
    
    if (cstBtn && astBtn) {
        cstBtn.addEventListener('click', () => {
            cstBtn.classList.add('active-toggle');
            astBtn.classList.remove('active-toggle');
            currentTreeMode = "CST";
            updateTreeDisplay();
        });
        
        astBtn.addEventListener('click', () => {
            astBtn.classList.add('active-toggle');
            cstBtn.classList.remove('active-toggle');
            currentTreeMode = "AST";
            updateTreeDisplay();
        });
    }

    function updateTreeDisplay() {
        if (!rawTreeText) {
            rdTreeContent.innerHTML = '<div class="empty-state">No parse tree generated. Click compile.</div>';
            return;
        }
        if (currentTreeMode === "AST") {
            const astText = convertCSTtoAST(rawTreeText);
            rdTreeContent.innerHTML = formatTree(astText);
        } else {
            rdTreeContent.innerHTML = formatTree(rawTreeText);
        }
    }

    function convertCSTtoAST(cstText) {
        const lines = cstText.split('\n');
        let astLines = [];
        const skipNodes = [
            "Type", "Expr", "Term", "Factor", "Designator_Tail", "FormPars_Opt", "FormPars_Tail", "VarDecl_Tail", "Designator",
            "KW_PROGRAM", "LBRACE", "RBRACE", "LPAREN", "RPAREN", "SEMICOLON", "ASSIGN", "COMMA", "DOT",
            "KW_VOID", "KW_FINAL", "KW_CLASS", "KW_IF", "KW_ELSE", "KW_WHILE", "KW_READ", "KW_PRINT", "KW_RETURN", "KW_NEW"
        ];
        let indentStack = [];
        
        for (let line of lines) {
            if (line.trim() === '') continue;
            
            const matchIndent = line.match(/^(│  )*/);
            const depth = matchIndent ? matchIndent[0].length / 3 : 0;
            
            const matchNode = line.match(/├── ([A-Za-z0-9_]+)/);
            if (!matchNode) {
                astLines.push(line);
                continue;
            }
            
            const nodeName = matchNode[1];
            
            while (indentStack.length > 0 && indentStack[indentStack.length - 1].depth >= depth) {
                indentStack.pop();
            }
            
            let currentAdjustment = indentStack.length > 0 ? indentStack[indentStack.length - 1].adj : 0;
            
            if (skipNodes.includes(nodeName)) {
                indentStack.push({ depth: depth, adj: currentAdjustment + 1 });
                continue;
            }
            
            const newDepth = Math.max(0, depth - currentAdjustment);
            const prefix = "│  ".repeat(newDepth);
            let content = line.substring(line.indexOf("├──"));
            
            // Clean up node labels for leaf values in AST representation
            content = content.replace(/├── IDENTIFIER \(token: "([^"]+)"\)/, '├── $1');
            content = content.replace(/├── NUMBER \(token: "([^"]+)"\)/, '├── $1');
            content = content.replace(/├── CHAR_CONST \(token: "([^"]+)"\)/, '├── $1');
            
            astLines.push(prefix + content);
        }
        
        return astLines.join('\n');
    }

    // Generates helpful, context-aware bug-fixing guidance
    function generateErrorGuide(message) {
        const msgLower = message.toLowerCase();
        if (msgLower.includes('expected semicolon') || msgLower.includes('expected ;') || msgLower.includes('expected \';\'')) {
            return "Make sure the current or preceding statement ends with a semicolon (<code>;</code>). Check for missing semicolons on declarations or assignments.";
        }
        if (msgLower.includes('expected =')) {
            return "A variable assignment statement was started, but the assignment operator (<code>=</code>) is missing. Check your assignment syntax.";
        }
        if (msgLower.includes('expected class') || msgLower.includes('expected program')) {
            return "A keyword is missing or typed incorrectly. Verify that declarations start with keywords such as <code>program</code> or <code>class</code>.";
        }
        if (msgLower.includes('expected number') || msgLower.includes('expected char constant')) {
            return "Constant declarations must be assigned directly to literal values. Verify that you have placed a valid <code>number</code> or <code>'character'</code> constant after the assignment <code>=</code>.";
        }
        if (msgLower.includes('expected relational') || msgLower.includes('expected relop')) {
            return "Conditional statements (like <code>if</code> or <code>while</code>) require a comparison operator between two expressions. Verify you are using <code>==</code>, <code>!=</code>, <code>&gt;</code>, <code>&gt;=</code>, <code>&lt;</code>, or <code>&lt;=</code>.";
        }
        if (msgLower.includes('expected \')\'') || msgLower.includes('expected )')) {
            return "Check for a missing closing parenthesis (<code>)</code>) in your statement. Parentheses around conditions or method parameter lists must be balanced.";
        }
        if (msgLower.includes('expected \'}\'') || msgLower.includes('expected }')) {
            return "Check for a missing closing brace (<code>}</code>). Program blocks, classes, and method bodies must start with <code>{</code> and close with <code>}</code>.";
        }
        if (msgLower.includes('unknown char') || msgLower.includes('lexical error')) {
            return "A character is used that is not supported by MicroJava's alphabet. Remove invalid symbols or correct typos.";
        }
        return "Verify syntax constraints, keyword definitions, and scopes. Check neighboring declarations for syntax anomalies.";
    }

    // ==========================================
    // MICROJAVA TO JAVASCRIPT TRANSPILER
    // ==========================================
    class MicroJavaTranspiler {
        constructor(tokens) {
            this.tokens = tokens;
            this.index = 0;
        }
        
        peek() {
            return this.tokens[this.index] || { type: 'EOF', lexeme: '' };
        }
        
        next() {
            return this.tokens[this.index++] || { type: 'EOF', lexeme: '' };
        }
        
        match(type) {
            if (this.peek().type === type) {
                return this.next();
            }
            return null;
        }
        
        parseProgram() {
            let js = "";
            
            this.match('KW_PROGRAM');
            const progName = this.next().lexeme;
            js += `// Transpiled Program: ${progName}\n\n`;
            
            while (this.peek().type !== 'LBRACE' && this.peek().type !== 'EOF') {
                const token = this.peek();
                if (token.type === 'KW_FINAL') {
                    js += this.parseConstDecl() + "\n";
                } else if (token.type === 'KW_CLASS') {
                    js += this.parseClassDecl() + "\n";
                } else {
                    js += this.parseVarDecl() + "\n";
                }
            }
            
            this.match('LBRACE');
            
            while (this.peek().type !== 'RBRACE' && this.peek().type !== 'EOF') {
                js += this.parseMethodDecl() + "\n";
            }
            
            this.match('RBRACE');
            return js;
        }
        
        parseConstDecl() {
            this.match('KW_FINAL');
            this.parseType(); 
            const name = this.next().lexeme;
            this.match('ASSIGN');
            const valToken = this.next(); 
            let val = valToken.lexeme;
            this.match('SEMICOLON');
            return `const ${name} = ${val};`;
        }
        
        parseClassDecl() {
            this.match('KW_CLASS');
            const className = this.next().lexeme;
            this.match('LBRACE');
            
            let fields = [];
            while (this.peek().type !== 'RBRACE' && this.peek().type !== 'EOF') {
                const type = this.parseType();
                let names = [];
                names.push(this.next().lexeme);
                while (this.match('COMMA')) {
                    names.push(this.next().lexeme);
                }
                this.match('SEMICOLON');
                
                let initVal = "null";
                if (type === 'int') initVal = "0";
                else if (type === 'char') initVal = "''";
                
                for (let name of names) {
                    fields.push({ name, initVal });
                }
            }
            this.match('RBRACE');
            
            let constructorBody = fields.map(f => `    this.${f.name} = ${f.initVal};`).join('\n');
            return `class ${className} {\n  constructor() {\n${constructorBody}\n  }\n}`;
        }
        
        parseVarDecl() {
            const type = this.parseType();
            let names = [];
            names.push(this.next().lexeme);
            while (this.match('COMMA')) {
                names.push(this.next().lexeme);
            }
            this.match('SEMICOLON');
            
            let initVal = "null";
            if (type === 'int') initVal = "0";
            else if (type === 'char') initVal = "''";
            
            return `let ${names.map(n => `${n} = ${initVal}`).join(', ')};`;
        }
        
        parseType() {
            const typeName = this.next().lexeme;
            if (this.match('LBRACKET')) {
                this.match('RBRACKET');
                return typeName + "[]";
            }
            return typeName;
        }
        
        parseMethodDecl() {
            const type = this.peek().type === 'KW_VOID' ? this.next().lexeme : this.parseType();
            const methodName = this.next().lexeme;
            
            this.match('LPAREN');
            let params = [];
            if (this.peek().type !== 'RPAREN') {
                this.parseType();
                params.push(this.next().lexeme);
                while (this.match('COMMA')) {
                    this.parseType();
                    params.push(this.next().lexeme);
                }
            }
            this.match('RPAREN');
            
            let localVars = "";
            while (this.peek().type !== 'LBRACE' && this.peek().type !== 'EOF') {
                localVars += "  " + this.parseVarDecl() + "\n";
            }
            
            const blockBody = this.parseBlockBody();
            return `async function ${methodName}(${params.join(', ')}) {\n${localVars}${blockBody}\n}`;
        }
        
        parseBlockBody() {
            this.match('LBRACE');
            let js = "";
            while (this.peek().type !== 'RBRACE' && this.peek().type !== 'EOF') {
                js += this.parseStatement() + "\n";
            }
            this.match('RBRACE');
            return js;
        }
        
        parseStatement() {
            const token = this.peek();
            
            if (token.type === 'KW_IF') {
                this.next();
                this.match('LPAREN');
                const cond = this.parseCondition();
                this.match('RPAREN');
                const stmt1 = this.parseStatement();
                let elsePart = "";
                if (this.match('KW_ELSE')) {
                    elsePart = " else " + this.parseStatement();
                }
                return `if (${cond}) ${stmt1}${elsePart}`;
            }
            
            if (token.type === 'KW_WHILE') {
                this.next();
                this.match('LPAREN');
                const cond = this.parseCondition();
                this.match('RPAREN');
                const stmt = this.parseStatement();
                return `while (${cond}) {\n  await yieldCheck();\n  ${stmt}\n}`;
            }
            
            if (token.type === 'KW_RETURN') {
                this.next();
                let expr = "";
                if (this.peek().type !== 'SEMICOLON') {
                    expr = " " + this.parseExpr();
                }
                this.match('SEMICOLON');
                return `return${expr};`;
            }
            
            if (token.type === 'KW_READ') {
                this.next();
                this.match('LPAREN');
                const dest = this.parseDesignator();
                this.match('RPAREN');
                this.match('SEMICOLON');
                return `${dest} = await window.__mj_read();`;
            }
            
            if (token.type === 'KW_PRINT') {
                this.next();
                this.match('LPAREN');
                const expr = this.parseExpr();
                let width = "";
                if (this.match('COMMA')) {
                    width = ", " + this.next().lexeme;
                }
                this.match('RPAREN');
                this.match('SEMICOLON');
                return `window.__mj_print(${expr}${width});`;
            }
            
            if (token.type === 'LBRACE') {
                return `{\n${this.parseBlockBody()}\n}`;
            }
            
            if (token.type === 'SEMICOLON') {
                this.next();
                return ";";
            }
            
            const dest = this.parseDesignator();
            if (this.match('ASSIGN')) {
                const expr = this.parseExpr();
                this.match('SEMICOLON');
                return `${dest} = ${expr};`;
            } else {
                const actPars = this.parseActPars();
                this.match('SEMICOLON');
                return `await ${dest}${actPars};`;
            }
        }
        
        parseCondition() {
            const left = this.parseExpr();
            const relop = this.next().lexeme; 
            const right = this.parseExpr();
            return `${left} ${relop} ${right}`;
        }
        
        parseExpr() {
            let neg = "";
            if (this.match('MINUS')) {
                neg = "-";
            }
            
            let term = this.parseTerm();
            let expr = neg + term;
            
            while (this.peek().type === 'PLUS' || this.peek().type === 'MINUS') {
                const op = this.next().lexeme;
                const nextTerm = this.parseTerm();
                expr += ` ${op} ${nextTerm}`;
            }
            return expr;
        }
        
        parseTerm() {
            let left = this.parseFactor();
            while (this.peek().type === 'STAR' || this.peek().type === 'SLASH' || this.peek().type === 'PERCENT') {
                const op = this.next().lexeme;
                const right = this.parseFactor();
                if (op === '/') {
                    left = `intDiv(${left}, ${right})`;
                } else if (op === '%') {
                    left = `intMod(${left}, ${right})`;
                } else {
                    left = `${left} ${op} ${right}`;
                }
            }
            return left;
        }
        
        parseFactor() {
            const token = this.peek();
            
            if (token.type === 'NUMBER') {
                return this.next().lexeme;
            }
            
            if (token.type === 'CHAR_CONST') {
                return this.next().lexeme; 
            }
            
            if (token.type === 'KW_NEW') {
                this.next();
                const className = this.next().lexeme;
                if (this.match('LBRACKET')) {
                    const expr = this.parseExpr();
                    this.match('RBRACKET');
                    let fillVal = "0";
                    if (className === 'char') fillVal = "''";
                    else if (className !== 'int') fillVal = "null";
                    return `new Array(${expr}).fill(${fillVal})`;
                }
                return `new ${className}()`;
            }
            
            if (token.type === 'LPAREN') {
                this.next();
                const expr = this.parseExpr();
                this.match('RPAREN');
                return `(${expr})`;
            }
            
            const dest = this.parseDesignator();
            if (this.peek().type === 'LPAREN') {
                const actPars = this.parseActPars();
                return `await ${dest}${actPars}`;
            }
            return dest;
        }
        
        parseDesignator() {
            let name = this.next().lexeme;
            while (this.peek().type === 'DOT' || this.peek().type === 'LBRACKET') {
                if (this.match('DOT')) {
                    const fieldName = this.next().lexeme;
                    name += `.${fieldName}`;
                } else if (this.match('LBRACKET')) {
                    const expr = this.parseExpr();
                    this.match('RBRACKET');
                    name += `[${expr}]`;
                }
            }
            return name;
        }
        
        parseActPars() {
            this.match('LPAREN');
            let args = [];
            if (this.peek().type !== 'RPAREN') {
                args.push(this.parseExpr());
                while (this.match('COMMA')) {
                    args.push(this.parseExpr());
                }
            }
            this.match('RPAREN');
            return `(${args.join(', ')})`;
        }
    }

    // ==========================================
    // INTERACTIVE REAL TERMINAL RUNNER ENGINE
    // ==========================================
    const runVmBtn = document.getElementById('run-vm-btn');
    const resetVmBtn = document.getElementById('reset-vm-btn');
    const vmLayout = document.getElementById('vm-layout');
    const vmLog = document.getElementById('vm-log');
    const vmInput = document.getElementById('vm-input');

    let inputResolver = null;
    let isProgramRunning = false;
    let loopCounter = 0;

    runVmBtn.addEventListener('click', startProgramExecution);
    resetVmBtn.addEventListener('click', stopProgramExecution);
    
    vmInput.addEventListener('keydown', (e) => {
        if (e.key === 'Enter') {
            const rawVal = vmInput.value;
            vmInput.value = '';
            
            if (inputResolver) {
                appendVmLog(rawVal + "\n", 'input');
                vmInput.disabled = true;
                const resolve = inputResolver;
                inputResolver = null;
                resolve(rawVal);
            }
        }
    });

    function appendVmLog(message, type) {
        let color = '#38bdf8'; // info cyan
        if (type === 'stdout') color = '#f3f4f6'; // print white
        else if (type === 'input') color = '#10b981'; // input green
        else if (type === 'success') color = '#34d399'; // success green
        else if (type === 'error') color = '#f87171'; // error red
        else if (type === 'info') color = '#60a5fa'; // info blue
        
        const formatted = escapeHtml(String(message));
        if (type === 'stdout') {
            vmLog.innerHTML += `<span style="color: ${color}; font-family: 'Fira Code', monospace; white-space: pre-wrap;">${formatted}</span>`;
        } else {
            vmLog.innerHTML += `<div style="color: ${color}; margin-top: 0.2rem; font-family: 'Fira Code', monospace; white-space: pre-wrap;">${formatted}</div>`;
        }
        vmLog.scrollTop = vmLog.scrollHeight;
    }

    async function startProgramExecution() {
        if (isProgramRunning) return;
        
        isProgramRunning = true;
        runVmBtn.style.display = 'none';
        resetVmBtn.style.display = 'inline-flex';
        vmLayout.style.display = 'grid';
        vmLog.innerHTML = '';
        vmInput.disabled = true;
        vmInput.value = '';
        
        appendVmLog("--- Terminal Console Started ---\n", "info");
        
        if (!currentTokens || currentTokens.length === 0) {
            appendVmLog("[Terminal Error] No compiled tokens found. Compile first.\n", "error");
            stopProgramExecution();
            return;
        }

        try {
            const transpiler = new MicroJavaTranspiler(currentTokens);
            const jsCode = transpiler.parseProgram();
            
            window.__mj_print = (val, width) => {
                let strVal = val;
                if (typeof val === 'object' && val !== null) {
                    strVal = JSON.stringify(val);
                } else if (val === null) {
                    strVal = "null";
                }
                if (width !== undefined) {
                    strVal = String(strVal).padStart(width);
                }
                appendVmLog(strVal, 'stdout');
            };
            
            window.__mj_read = async () => {
                vmInput.disabled = false;
                vmInput.placeholder = "Enter input and press Enter...";
                vmInput.focus();
                
                return new Promise((resolve) => {
                    inputResolver = (rawVal) => {
                        vmInput.placeholder = "Enter input here...";
                        if (/^-?\d+$/.test(rawVal)) {
                            resolve(parseInt(rawVal, 10));
                        } else {
                            resolve(rawVal);
                        }
                    };
                });
            };

            loopCounter = 0;
            const fullRunnerCode = `
(async () => {
    function chr(x) { return String.fromCharCode(x); }
    function ord(c) {
        if (typeof c === 'string' && c.length > 0) return c.charCodeAt(0);
        if (typeof c === 'number') return c;
        return 0;
    }
    function len(a) { return a ? a.length : 0; }
    function intDiv(a, b) {
        if (b === 0) throw new Error("Division by zero");
        return Math.trunc(a / b);
    }
    function intMod(a, b) {
        if (b === 0) throw new Error("Division by zero (modulo)");
        return a % b;
    }
    
    async function yieldCheck() {
        loopCounter++;
        if (loopCounter % 500 === 0) {
            await new Promise(resolve => setTimeout(resolve, 0));
        }
    }
    
    try {
        ${jsCode}
        if (typeof main === 'function') {
            await main();
            appendVmLog("\\n[Program finished successfully]\\n", "success");
        } else {
            appendVmLog("[VM Error] main() method not found!\\n", "error");
        }
    } catch (err) {
        appendVmLog("\\n[Runtime Error]: " + err.message + "\\n", "error");
    } finally {
        isProgramRunning = false;
        runVmBtn.style.display = 'inline-flex';
        resetVmBtn.style.display = 'none';
        vmInput.disabled = true;
        vmInput.placeholder = "Enter input here...";
    }
})();
            `;
            
            eval(fullRunnerCode);
            
        } catch (transpileErr) {
            appendVmLog(`[Transpiler Error] Failed to prepare execution: ${transpileErr.message}\n`, "error");
            stopProgramExecution();
        }
    }

    function stopProgramExecution() {
        isProgramRunning = false;
        inputResolver = null;
        vmInput.disabled = true;
        vmInput.value = '';
        vmInput.placeholder = "Program terminated.";
        
        appendVmLog("\n--- Program Terminated ---\n", "info");
        
        runVmBtn.style.display = 'inline-flex';
        resetVmBtn.style.display = 'none';
    }

    // Helper to escape HTML tags
    function escapeHtml(text) {
        return text
            .replace(/&/g, "&amp;")
            .replace(/</g, "&lt;")
            .replace(/>/g, "&gt;")
            .replace(/"/g, "&quot;")
            .replace(/'/g, "&#039;");
    }
});
