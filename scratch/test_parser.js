const fs = require('fs');

// Mock data from the server response
const data = {
  "status": "success", 
  "stdout": "\u001b[1;36m==============================================================================\u001b[0m\n\u001b[1;36m                      PHASE 1: LEXICAL ANALYSIS (LEXER)                       \u001b[0m\n\u001b[1;36m==============================================================================\u001b[0m\nScanning source file: \u001b[1;33mtest\\temp.mj\u001b[0m...\n\n+----------------------+----------------------------+---------+---------+\n| Token Type           | Lexeme                     | Line    | Column  |\n+----------------------+----------------------------+---------+---------+\n| KW_PROGRAM           | program                    | 1       | 1       |\n| IDENTIFIER           | P                          | 1       | 9       |\n| LBRACE               | {                          | 1       | 11      |\n| RBRACE               | }                          | 1       | 12      |\n| EOF                  | EOF                        | 1       | 13      |\n+----------------------+----------------------------+---------+---------+\n\u001b[1;32m[SUCCESS] Lexical scan completed with 0 lexical errors. Total tokens: 5\u001b[0m\n\n\u001b[1;36m==============================================================================\u001b[0m\n\u001b[1;36m                   PHASE 2: RECURSIVE DESCENT PARSE TREE                      \u001b[0m\n\u001b[1;36m==============================================================================\u001b[0m\n\u251c\u2500\u2500 \u001b[1;33mProgram\u001b[0m (token: \"program\")\n------------------------------------------------------------------------------\n\u001b[1;32m[SUCCESS] Recursive Descent Parsing completed with 0 errors.\u001b[0m\n\n\u001b[1;36m==============================================================================\u001b[0m\n\u001b[1;36m                         PHASE 3: SYMBOL TABLE DUMP                           \u001b[0m\n\u001b[1;36m==============================================================================\u001b[0m\n+-------+----------------------+------------+----------------------------+------+\n| Scope | Symbol Name          | Kind       | Data Type                  | Line |\n+-------+----------------------+------------+----------------------------+------+\n| 0     | len                  | METHOD     | int                        | 0    |\n| 0     | ord                  | METHOD     | int                        | 0    |\n| 0     | chr                  | METHOD     | char                       | 0    |\n+-------+----------------------+------------+----------------------------+------+\n\n\u001b[1;36m==============================================================================\u001b[0m\n\u001b[1;36m                   PHASE 4: LL(1) PREDICTIVE PARSER STEPS                     \u001b[0m\n\u001b[1;36m==============================================================================\u001b[0m\nApplied Rule: \u001b[1;35mProgram\u001b[0m -> program ident Program_Body { MethodDecls } \nApplied Rule: \u001b[1;35mProgram_Body\u001b[0m -> \u001b[3mepsilon\u001b[0m\nApplied Rule: \u001b[1;35mMethodDecls\u001b[0m -> \u001b[3mepsilon\u001b[0m\n------------------------------------------------------------------------------\n\u001b[1;32m[SUCCESS] LL(1) Parser completed with 0 errors.\u001b[0m\n\n\u001b[1;36m==============================================================================\u001b[0m\n\u001b[1;36m                   PHASE 5: CANONICAL LR(1) PARSER ACTIONS                    \u001b[0m\n\u001b[1;36m==============================================================================\u001b[0m\nAction: \u001b[1;34mSHIFT\u001b[0m to State 1 (Token: \"program\")\nAction: \u001b[1;34mSHIFT\u001b[0m to State 3 (Token: \"P\")\nAction: \u001b[1;35mREDUCE\u001b[0m by rule \u001b[1;33mProgram_Body\u001b[0m (length 0)\n  -> GOTO State 5\nAction: \u001b[1;34mSHIFT\u001b[0m to State 13 (Token: \"{\")\nAction: \u001b[1;35mREDUCE\u001b[0m by rule \u001b[1;33mMethodDecls\u001b[0m (length 0)\n  -> GOTO State 25\nAction: \u001b[1;34mSHIFT\u001b[0m to State 33 (Token: \"}\")\nAction: \u001b[1;35mREDUCE\u001b[0m by rule \u001b[1;33mProgram\u001b[0m (length 6)\n  -> GOTO State 2\nAction: \u001b[1;32mACCEPT\u001b[0m\n------------------------------------------------------------------------------\n\u001b[1;32m[SUCCESS] Canonical LR(1) Parser completed with 0 errors.\u001b[0m\n\n", 
  "stderr": "", 
  "exit_code": 0
};

function stripAnsi(str) {
    return str.replace(/[\u001b\u009b][[()#;?]*(?:[0-9]{1,4}(?:;[0-9]{0,4})*)?[0-9A-ORZcf-nqry=><]/g, '');
}

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
    // Modified to support optional carriage returns \r
    return subText.replace(/^[\s\S]*?===+\r?\n/, '').trim();
}

function parseLexerTokens(text) {
    const rows = text.split('\n');
    let tbodyHtml = '';
    let tokenCount = 0;

    for (let row of rows) {
        const regex = /\|\s*(.*?)\s*\|\s*(.*?)\s*\|\s*(\d+)\s*\|\s*(\d+)\s*\|/;
        const match = row.match(regex);
        
        if (match) {
            const type = match[1];
            const lexeme = match[2];
            const line = match[3];
            const col = match[4];
            
            tbodyHtml += `<tr><td>${type}</td><td>${lexeme}</td><td>${line}</td><td>${col}</td></tr>\n`;
            tokenCount++;
        }
    }
    return { html: tbodyHtml, count: tokenCount };
}

try {
    console.log("Starting parsing test...");
    const cleanStdout = stripAnsi(data.stdout);
    
    console.log("\n--- Testing Lexer Extraction ---");
    const lexerSection = extractSection(cleanStdout, 'PHASE 1: LEXICAL ANALYSIS (LEXER)', 'PHASE 2: RECURSIVE DESCENT PARSE TREE');
    console.log("Lexer Section Length:", lexerSection ? lexerSection.length : "NULL");
    if (lexerSection) {
        const result = parseLexerTokens(lexerSection);
        console.log("Scanned Tokens Count:", result.count);
        console.log("First 3 lines of output html:\n", result.html.split('\n').slice(0, 3).join('\n'));
    }

    console.log("\n--- Testing RD Tree Extraction ---");
    const rdTreeSection = extractSection(cleanStdout, 'PHASE 2: RECURSIVE DESCENT PARSE TREE', 'PHASE 3: SYMBOL TABLE DUMP');
    console.log("RD Tree Section Length:", rdTreeSection ? rdTreeSection.length : "NULL");
    console.log("RD Tree Content:\n", rdTreeSection);

    console.log("\nParsing test completed successfully!");
} catch (e) {
    console.error("Test failed with error:", e);
}
