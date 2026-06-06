import re

# Read compiler run output
with open("scratch/compiler_simplesum_run.txt", "r", encoding="utf-8") as f:
    raw_content = f.read()

# Strip ANSI codes
def strip_ansi(text):
    ansi_escape = re.compile(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')
    return ansi_escape.sub('', text)

text = strip_ansi(raw_content)

# Let's find index positions of headers
p1_match = re.search(r"PHASE 1:[^\n]+", text)
p2_match = re.search(r"PHASE 2:[^\n]+", text)
p3_match = re.search(r"PHASE 3:[^\n]+", text)
p4_match = re.search(r"PHASE 4:[^\n]+", text)
p5_match = re.search(r"PHASE 5:[^\n]+", text)

# Helper to clean up any trailing border lines
def clean_section(sect_text):
    lines = sect_text.strip().splitlines()
    while lines:
        last = lines[-1].strip()
        if not last or all(c in "=-" for c in last):
            lines.pop()
        else:
            break
    while lines:
        first = lines[0].strip()
        if not first or all(c in "=-" for c in first) or first.startswith("Scanning source file"):
            lines.pop(0)
        else:
            break
    return "\n".join(lines).strip()

# Extract Phase 1-3 as normal
p1_start = text.find("\n", p1_match.end())
p1_start = text.find("\n", p1_start + 1)
p2_start_header = text.rfind("===", 0, p2_match.start())
p1_text = clean_section(text[p1_start:p2_start_header])

p2_start = text.find("\n", p2_match.end())
p2_start = text.find("\n", p2_start + 1)
p3_start_header = text.rfind("===", 0, p3_match.start())
p2_text = clean_section(text[p2_start:p3_start_header])

p3_start = text.find("\n", p3_match.end())
p3_start = text.find("\n", p3_start + 1)
p4_start_header = text.rfind("===", 0, p4_match.start())
p3_text = clean_section(text[p3_start:p4_start_header])

# Extract Phase 4 and extract only Action
p4_start = text.find("\n", p4_match.end())
p4_start = text.find("\n", p4_start + 1)
p5_start_header = text.rfind("===", 0, p5_match.start())
p4_raw = clean_section(text[p4_start:p5_start_header])

p4_actions = []
for line in p4_raw.splitlines():
    if "[LL_STEP]" in line:
        action_idx = line.find("| Action:")
        if action_idx != -1:
            p4_actions.append(line[action_idx + len("| Action:"):].strip())
        else:
            p4_actions.append(line.strip())
    else:
        p4_actions.append(line.strip())
p4_text = "\n".join(p4_actions)

# Extract Phase 5 and extract only Action
p5_start = text.find("\n", p5_match.end())
p5_start = text.find("\n", p5_start + 1)
p5_raw = clean_section(text[p5_start:])

p5_actions = []
for line in p5_raw.splitlines():
    if "[LR_STEP]" in line:
        action_idx = line.find("| Action:")
        if action_idx != -1:
            p5_actions.append(line[action_idx + len("| Action:"):].strip())
        else:
            p5_actions.append(line.strip())
    else:
        p5_actions.append(line.strip())
p5_text = "\n".join(p5_actions)

print(f"P1 len: {len(p1_text)}")
print(f"P2 len: {len(p2_text)}")
print(f"P3 len: {len(p3_text)}")
print(f"P4 len: {len(p4_text)}")
print(f"P5 len: {len(p5_text)}")

# Let's save these to a file to verify
with open("scratch/processed_simplesum.txt", "w", encoding="utf-8") as f:
    f.write(f"=== P1 ===\n{p1_text}\n\n=== P2 ===\n{p2_text}\n\n=== P3 ===\n{p3_text}\n\n=== P4 ===\n{p4_text}\n\n=== P5 ===\n{p5_text}\n")
