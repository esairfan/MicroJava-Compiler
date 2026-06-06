import re

with open("scratch/compiler_stripped.txt", "r", encoding="utf-8") as f:
    text = f.read()

# Let's find index positions of headers
p1_match = re.search(r"PHASE 1:[^\n]+", text)
p2_match = re.search(r"PHASE 2:[^\n]+", text)
p3_match = re.search(r"PHASE 3:[^\n]+", text)
p4_match = re.search(r"PHASE 4:[^\n]+", text)
p5_match = re.search(r"PHASE 5:[^\n]+", text)

print("P1 Match:", p1_match.group(0) if p1_match else "None")
print("P2 Match:", p2_match.group(0) if p2_match else "None")
print("P3 Match:", p3_match.group(0) if p3_match else "None")
print("P4 Match:", p4_match.group(0) if p4_match else "None")
print("P5 Match:", p5_match.group(0) if p5_match else "None")

# Now let's extract
# Phase 1: starts after the === line under p1 header, ends before p2 header block
p1_start = text.find("\n", p1_match.end())
# skip the separator line of '='
p1_start = text.find("\n", p1_start + 1)

p2_start_header = text.rfind("===", 0, p2_match.start())
p1_text = text[p1_start:p2_start_header].strip()

# Phase 2: starts after the === line under p2 header, ends before p3 header block
p2_start = text.find("\n", p2_match.end())
p2_start = text.find("\n", p2_start + 1)
p3_start_header = text.rfind("===", 0, p3_match.start())
p2_text = text[p2_start:p3_start_header].strip()

# Phase 3: starts after the === line under p3 header, ends before p4 header block
p3_start = text.find("\n", p3_match.end())
p3_start = text.find("\n", p3_start + 1)
p4_start_header = text.rfind("===", 0, p4_match.start())
p3_text = text[p3_start:p4_start_header].strip()

# Phase 4: starts after the === line under p4 header, ends before p5 header block
p4_start = text.find("\n", p4_match.end())
p4_start = text.find("\n", p4_start + 1)
p5_start_header = text.rfind("===", 0, p5_match.start())
p4_text = text[p4_start:p5_start_header].strip()

# Phase 5: starts after the === line under p5 header, ends before the end of the file or SUCCESS line
p5_start = text.find("\n", p5_match.end())
p5_start = text.find("\n", p5_start + 1)
p5_text = text[p5_start:].strip()

print(f"P1 len: {len(p1_text)}")
print(f"P2 len: {len(p2_text)}")
print(f"P3 len: {len(p3_text)}")
print(f"P4 len: {len(p4_text)}")
print(f"P5 len: {len(p5_text)}")

# Let's print snippets to verify
print("P1 snippet:")
print("\n".join(p1_text.splitlines()[:5]))
print("...")
print("\n".join(p1_text.splitlines()[-3:]))

print("\nP3 snippet:")
print(p3_text)
