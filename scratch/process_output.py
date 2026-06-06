import re

def strip_ansi(text):
    ansi_escape = re.compile(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')
    return ansi_escape.sub('', text)

with open("scratch/compiler_temp_run.txt", "r", encoding="utf-8") as f:
    content = f.read()

stripped = strip_ansi(content)

with open("scratch/compiler_stripped.txt", "w", encoding="utf-8") as f:
    f.write(stripped)

print("Stripped ANSI escapes. Saved to scratch/compiler_stripped.txt")
