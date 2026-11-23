"""Summarize actual LAB response events, never synthesize measurements."""
import argparse
import csv
import re
import statistics
from pathlib import Path

PATTERN = re.compile(r'LAB event=response id=(\d+) csp=([01]) response_ms=([\d.]+)')

def extract(path):
    for line in path.read_text(encoding='utf-8', errors='replace').splitlines():
        match = PATTERN.search(line)
        if match:
            yield {'log': path.name, 'id': int(match[1]), 'csp': int(match[2]), 'response_ms': float(match[3])}

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('logs', type=Path, nargs='+', help='Client logs from Saved/Runs')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    if args.output.exists():
        parser.error('Output exists; choose a new path to preserve previous measurements')
    rows = [row for path in args.logs for row in extract(path)]
    if not rows:
        parser.error('No measured response events found; perform trials in Unreal first')
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open('w', newline='', encoding='utf-8-sig') as stream:
        writer = csv.DictWriter(stream, fieldnames=['log', 'id', 'csp', 'response_ms'])
        writer.writeheader()
        writer.writerows(rows)
    for mode in [0, 1]:
        values = sorted(row['response_ms'] for row in rows if row['csp'] == mode)
        if values:
            print(f'CSP={mode}: n={len(values)}, median={statistics.median(values):.2f} ms, '
                  f'min={min(values):.2f}, max={max(values):.2f}')

if __name__ == '__main__':
    main()
