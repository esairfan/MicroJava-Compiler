import pypdf

def extract_text(pdf_path):
    try:
        reader = pypdf.PdfReader(pdf_path)
        return "\n".join(page.extract_text() for page in reader.pages)
    except Exception as e:
        return f"Error reading {pdf_path}: {e}"

with open("requirements.txt", "w", encoding="utf-8") as f:
    f.write("--- MicroJava.pdf ---\n")
    f.write(extract_text("MicroJava.pdf"))
    f.write("\n\n--- MiniCompilerProject.pdf ---\n")
    f.write(extract_text("MiniCompilerProject.pdf"))
