"""Archive the actual packaged Windows game without optional debug symbols."""
import argparse
import hashlib
import shutil
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--engine', type=Path, default=Path('F:/Epic Games/UE_5.8'))
    args = parser.parse_args()
    base = ROOT / 'dist/Windows'
    if not (base / 'PredictionLab.exe').exists():
        parser.error('Run package_windows.ps1 first')
    redist = base / 'Engine/Extras/Redist/en-us'
    redist.mkdir(parents=True, exist_ok=True)
    for name in ['vc_redist.x64.exe', 'GameInputRedist.msi']:
        shutil.copy2(args.engine / 'Engine/Extras/Redist/en-us' / name, redist / name)
    output = ROOT / 'dist/PredictionLab_Windows.zip'
    files = [p for p in base.rglob('*') if p.is_file() and p.suffix.lower() != '.pdb'
             and 'Saved' not in p.relative_to(base).parts]
    with zipfile.ZipFile(output, 'w', compression=zipfile.ZIP_DEFLATED, compresslevel=6) as archive:
        for path in sorted(files):
            archive.write(path, Path('PredictionLab_Windows') / path.relative_to(base))
    with zipfile.ZipFile(output) as archive:
        if archive.testzip():
            raise RuntimeError('Archive CRC failure')
        assert 'PredictionLab_Windows/PredictionLab.exe' in archive.namelist()
        assert 'PredictionLab_Windows/Start_Lag100.bat' in archive.namelist()
    sha = hashlib.file_digest(output.open('rb'), 'sha256').hexdigest()
    (ROOT / 'docs/media/portable_sha256.txt').write_text(f'{sha}  PredictionLab_Windows.zip\n', encoding='utf-8')
    print(f'{output}: {output.stat().st_size / 1e6:.1f} MB, {len(files)} files, CRC OK\nSHA256 {sha}')

if __name__ == '__main__':
    main()
