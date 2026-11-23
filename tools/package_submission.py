"""Package source and documents; include the generated Unreal map if present."""
from pathlib import Path
import zipfile

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / 'Prac_4.zip'

def main():
    files = [ROOT / name for name in ['README.md', '.gitignore', 'PredictionLab.uproject']]
    for directory in ['Source', 'Config', 'Content', 'tools', 'docs']:
        base = ROOT / directory
        if base.exists():
            files.extend(p for p in base.rglob('*') if p.is_file() and '__pycache__' not in p.parts)
    with zipfile.ZipFile(OUTPUT, 'w', compression=zipfile.ZIP_DEFLATED) as archive:
        for path in sorted(files):
            archive.write(path, Path('prac_4') / path.relative_to(ROOT))
    with zipfile.ZipFile(OUTPUT) as archive:
        bad = archive.testzip()
        if bad:
            raise RuntimeError('Archive CRC failure: ' + bad)
        print(f'{OUTPUT.name}: {len(archive.namelist())} files, CRC OK')
    if not (ROOT / 'Content/Maps/PredictionArena.umap').exists():
        print('Map is not generated yet: run tools/build.ps1 on a machine with Unreal Engine.')

if __name__ == '__main__':
    main()
