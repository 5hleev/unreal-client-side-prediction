"""Export the plain-text defense cheat sheet to a printable PDF."""
import html
import re
from pathlib import Path
import pymupdf

ROOT = Path(__file__).resolve().parents[1]

def main():
    source = ROOT / 'docs/Шпаргалка_ПР4.txt'
    output = ROOT / 'docs/Шпаргалка_ПР4.pdf'
    content = []
    for i, block in enumerate(source.read_text(encoding='utf-8').split('\n\n')):
        block = block.strip()
        if not block:
            continue
        tag = 'h1' if i == 0 else ('h2' if re.match(r'^\d+\. ', block) and '\n' not in block else 'p')
        content.append(f'<{tag}>' + html.escape(block).replace('\n', '<br>') + f'</{tag}>')
    css = '''
    @font-face {font-family: Arial; src: url(arial.ttf);}
    @font-face {font-family: Arial; src: url(arialbd.ttf); font-weight: bold;}
    body {font-family: Arial; font-size: 9.5pt; line-height: 1.18;}
    h1 {font-size: 14pt;} h2 {font-size: 11pt; margin-top: 12pt; margin-bottom: 5pt;}
    p {margin-top: 0; margin-bottom: 7pt;}
    '''
    story = pymupdf.Story(html=''.join(content), user_css=css, archive=pymupdf.Archive('C:/Windows/Fonts'))
    writer = pymupdf.DocumentWriter(str(output))
    page = pymupdf.paper_rect('a4')
    more = True
    while more:
        device = writer.begin_page(page)
        more, _ = story.place(page + (38, 32, -38, -32))
        story.draw(device)
        writer.end_page()
    writer.close()
    with pymupdf.open(output) as document:
        for i, p in enumerate(document):
            p.insert_text((page.width - 65, page.height - 16), f'{i+1} / {len(document)}', fontsize=8)
        document.saveIncr()
        print(f'{output.name}: {len(document)} pages')

if __name__ == '__main__':
    main()
