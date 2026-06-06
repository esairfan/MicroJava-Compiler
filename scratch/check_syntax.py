import sys
import re

def check_syntax(filename):
    with open(filename, 'r', encoding='utf-8') as f:
        code = f.read()

    # Step 1: Remove comments
    # Multi-line comments: /* ... */
    # Single-line comments: // ...
    def remove_comments(text):
        # Match single-line comments and multi-line comments
        # and replace them, preserving newlines so line numbers don't change.
        def replacer(match):
            s = match.group(0)
            if s.startswith('/'):
                return '\n' * s.count('\n')
            else:
                return s
        pattern = re.compile(
            r'//.*?$|/\*.*?\*/|\'(?:\\.|[^\\\'])*\'|"(?:\\.|[^\\"])*"',
            re.DOTALL | re.MULTILINE
        )
        return re.sub(pattern, replacer, text)

    # Step 2: Since we removed string literals in comment pattern if they are just strings,
    # let's be more precise. Let's write a character parser.

    # Character-based parser to strip comments, strings, regexes
    cleaned = []
    in_string = None
    escape = False
    in_comment = None  # 'single', 'multi'
    in_regex = False
    
    i = 0
    n = len(code)
    
    # We want to replace all comments, strings, and regexes with spaces/newlines
    # so that they don't contain any {}, (), [] but preserve structural characters.
    while i < n:
        char = code[i]
        
        if escape:
            cleaned.append(' ')
            escape = False
            i += 1
            continue
            
        if in_comment == 'single':
            if char == '\n':
                in_comment = None
                cleaned.append('\n')
            else:
                cleaned.append(' ')
            i += 1
            continue
            
        if in_comment == 'multi':
            if char == '*' and i + 1 < n and code[i+1] == '/':
                in_comment = None
                cleaned.append('  ')
                i += 2
            elif char == '\n':
                cleaned.append('\n')
                i += 1
            else:
                cleaned.append(' ')
                i += 1
            continue
            
        if in_string:
            if char == '\\':
                escape = True
                cleaned.append(' ')
            elif char == in_string:
                in_string = None
                cleaned.append(' ')
            elif char == '\n':
                cleaned.append('\n')
            else:
                cleaned.append(' ')
            i += 1
            continue

        if in_regex:
            if char == '\\':
                escape = True
                cleaned.append(' ')
            elif char == '/':
                in_regex = False
                cleaned.append(' ')
            elif char == '\n':
                # regex cannot contain raw newline usually, but just in case
                cleaned.append('\n')
            else:
                cleaned.append(' ')
            i += 1
            continue

        # Check single-line comment
        if char == '/' and i + 1 < n and code[i+1] == '/':
            in_comment = 'single'
            cleaned.append('  ')
            i += 2
            continue
            
        # Check multi-line comment
        if char == '/' and i + 1 < n and code[i+1] == '*':
            in_comment = 'multi'
            cleaned.append('  ')
            i += 2
            continue

        # Check regex: in JS, regexes can start after assignments (=, :), return, operators, or start of block
        # To be safe, if we see a '/' and there is a '/' later on the same line, and it is not division
        # Let's see if the previous non-space token allows a regex.
        if char == '/':
            # Check if this is a division or a regex.
            # Look backwards at non-space chars
            j = len(cleaned) - 1
            prev_char = ''
            while j >= 0:
                if cleaned[j].strip():
                    prev_char = cleaned[j]
                    break
                j -= 1
            # If prev_char is an operator, assignment, open bracket, comma, or empty, it's a regex.
            # (e.g. '=', '(', ',', ':', '[', '?', '&', '|', '!', 'return')
            # Let's print out what we see
            is_regex = False
            if prev_char in ['=', '(', ',', ':', '[', '?', '&', '|', '!', '', ';', '{', '}']:
                is_regex = True
            elif prev_char == 't': # check if 'return'
                # check if 'return' precedes
                # simplified: let's assume it's regex unless it's preceded by alphanumeric or close brackets/parens/quotes
                pass
            
            # Let's use a simpler heuristic: if a slash is preceded by an alphanumeric character or ')' or ']', it is likely division.
            # Otherwise, it is a regex.
            if prev_char.isalnum() or prev_char in [')', ']', '"', "'", '`']:
                is_regex = False
            else:
                is_regex = True
                
            if is_regex:
                in_regex = True
                cleaned.append(' ')
                i += 1
                continue

        if char in ['"', "'", '`']:
            in_string = char
            cleaned.append(' ')
            i += 1
            continue
            
        cleaned.append(char)
        i += 1

    cleaned_code = "".join(cleaned)
    
    # Now check syntax on cleaned_code
    stack = []
    line_num = 1
    col_num = 0
    
    for i, char in enumerate(cleaned_code):
        if char == '\n':
            line_num += 1
            col_num = 0
        else:
            col_num += 1
            
        if char in ['{', '(', '[']:
            stack.append((char, line_num, col_num))
        elif char in ['}', ')', ']']:
            if not stack:
                print(f"Mismatched closing '{char}' at line {line_num}, col {col_num}")
            else:
                open_char, o_line, o_col = stack[-1]
                matches = {'}': '{', ')': '(', ']': '['}
                if matches[char] == open_char:
                    stack.pop()
                else:
                    print(f"Mismatched closing '{char}' at line {line_num}, col {col_num}. Expected match for '{open_char}' from line {o_line}, col {o_col}")
                    stack.pop()
                    
    if stack:
        print("Remaining open elements in stack:")
        for char, line, col in stack:
            print(f"  '{char}' at line {line}, col {col}")
    else:
        print("All brackets, braces, and parentheses are balanced!")

if __name__ == '__main__':
    check_syntax('gui/app.js')
