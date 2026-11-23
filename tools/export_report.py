"""Export repository documentation to Word and PDF, preserving actual verification status."""
from pathlib import Path
import html
import re
from docx import Document
from docx.shared import Cm, Pt
from docx.enum.text import WD_ALIGN_PARAGRAPH
import pymupdf

ROOT = Path(__file__).resolve().parents[1]
DOCS = ROOT / 'docs'
SOURCES = ['Prediction_Implementation.md', 'Results.md', 'Evidence.md', 'Prediction_Test_Matrix.md', 'Portable_Guide.md', 'Launch_and_Defense.md', 'Defense.md', 'Verification.md']

def plain(text):
    return re.sub(r'\[([^]]+)\]\([^)]+\)', r'\1', text).replace('**', '').replace('`', '')

def blocks(source):
    lines = source.read_text(encoding='utf-8').splitlines()
    i = 0
    while i < len(lines):
        line = lines[i].strip()
        i += 1
        if not line:
            continue
        picture = re.fullmatch(r'!\[([^]]*)\]\(([^)]+)\)', line)
        if picture:
            yield 'image', (picture[1], picture[2])
        elif line.startswith('|'):
            rows = []
            while True:
                cells = [plain(c.strip()) for c in line.strip('|').split('|')]
                if not all(re.fullmatch(r':?-+:?', c) for c in cells):
                    rows.append(cells)
                if i >= len(lines) or not lines[i].strip().startswith('|'):
                    break
                line = lines[i].strip()
                i += 1
            yield 'table', rows
        elif line.startswith('#'):
            depth = len(line) - len(line.lstrip('#'))
            yield f'h{min(depth, 3)}', plain(line.lstrip('#').strip())
        else:
            parts = [line]
            while i < len(lines) and lines[i].strip() and not lines[i].startswith(('#', '|', '- ')):
                parts.append(lines[i].strip())
                i += 1
            yield 'p', plain(' '.join(parts))

def main():
    doc = Document()
    section = doc.sections[0]
    section.top_margin = section.bottom_margin = Cm(2)
    section.left_margin = Cm(2.5)
    section.right_margin = Cm(1.5)
    normal = doc.styles['Normal']
    normal.font.name = 'Times New Roman'
    normal.font.size = Pt(12)
    normal.paragraph_format.space_after = Pt(6)
    title = 'Практическая работа №4'
    subtitle = 'Client-Side Prediction и Server Reconciliation в Unreal Engine'
    status = 'UE 5.8.4: проект собран. Реальные сетевые испытания и ограничения приведены в отчёте.'
    for text in [title, subtitle, 'Шлеев Максим Евгеньевич, ИКБО-21-23', status]:
        p = doc.add_paragraph(text)
        p.alignment = WD_ALIGN_PARAGRAPH.CENTER
    content = [f'<h1>{title}</h1><p>{subtitle}</p><p>Шлеев Максим Евгеньевич, ИКБО-21-23</p><p><b>{status}</b></p>']
    for name in SOURCES:
        doc.add_page_break()
        content.append('<div class="section">')
        for kind, value in blocks(DOCS / name):
            if kind == 'image':
                caption, filename = value
                doc.add_picture(str(DOCS / filename), width=Cm(16))
                doc.add_paragraph(caption)
                content.append(f'<img src="{html.escape(filename)}" style="width:100%"><p>{html.escape(caption)}</p>')
            elif kind == 'table':
                table = doc.add_table(rows=0, cols=len(value[0]))
                table.style = 'Table Grid'
                content.append('<table>')
                for n, row in enumerate(value):
                    cells = table.add_row().cells
                    tag = 'th' if n == 0 else 'td'
                    content.append('<tr>')
                    for cell, text in zip(cells, row):
                        cell.text = text
                        for p in cell.paragraphs:
                            for run in p.runs:
                                run.font.size = Pt(8 if len(row) > 5 else 10)
                                run.bold = n == 0
                        content.append(f'<{tag}>{html.escape(text)}</{tag}>')
                    content.append('</tr>')
                content.append('</table>')
            else:
                if kind.startswith('h'):
                    doc.add_heading(value, int(kind[1]))
                else:
                    doc.add_paragraph(value)
                content.append(f'<{kind}>{html.escape(value)}</{kind}>')
        content.append('</div>')
    doc.save(DOCS / 'ПР4_Шлеев_ИКБО-21-23.docx')
    css = '''
    @font-face {font-family: Arial; src: url(arial.ttf);}
    @font-face {font-family: Arial; src: url(arialbd.ttf); font-weight: bold;}
    body {font-family: Arial; font-size: 10pt; line-height: 1.35;}
    h1 {font-size: 18pt;} h2 {font-size: 14pt;} h3 {font-size: 12pt;}
    .section {page-break-before: always;}
    table {border-collapse: collapse; width: 100%; font-size: 8pt;}
    td, th {border: 0.5pt solid #888; padding: 4pt; vertical-align: top;}
    th {background-color: #e6edf5;}
    '''
    archive = pymupdf.Archive('C:/Windows/Fonts')
    archive.add(str(DOCS))
    story = pymupdf.Story(html=''.join(content), user_css=css, archive=archive)
    writer = pymupdf.DocumentWriter(str(DOCS / 'ПР4_Шлеев_ИКБО-21-23.pdf'))
    page = pymupdf.paper_rect('a4')
    more = True
    while more:
        device = writer.begin_page(page)
        more, _ = story.place(page + (45, 40, -40, -40))
        story.draw(device)
        writer.end_page()
    writer.close()
    print('Word and PDF exported')

if __name__ == '__main__':
    main()
