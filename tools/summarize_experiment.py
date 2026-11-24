"""Summarize completed real Unreal network runs, including correction evidence."""
import argparse
import re
import statistics
from pathlib import Path
from analyze_logs import extract

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('run', type=Path)
    args = parser.parse_args()
    lines = ['# Результаты реальных сетевых испытаний UE 5.8.4', '',
             f'Исходные данные: `{args.run.name}`. Два процесса Unreal: listen server и клиент.', '',
             '| Профиль | CSP | n | Медиана, мс | Min, мс | Max, мс |',
             '|---|---|---|---|---|---|']
    evidence = []
    for directory in sorted(args.run.iterdir()):
        client = directory / 'client.log'
        if not client.exists():
            continue
        text = client.read_text(encoding='utf-8', errors='replace')
        host = (directory / 'host.log').read_text(encoding='utf-8', errors='replace')
        if 'LAB event=auto_complete' not in text:
            raise RuntimeError(f'Incomplete run: {directory}')
        samples = list(extract(client))
        for csp in (0, 1):
            # Trial 20 deliberately rejects the action; 21-22 are recovery trials.
            # Keep only the first ten trials of each mode for the paired comparison.
            values = [s['response_ms'] for s in samples if s['csp'] == csp and s['id'] <= 40]
            if values:
                lines.append(f'| {directory.name} | {csp} | {len(values)} | {statistics.median(values):.2f} | {min(values):.2f} | {max(values):.2f} |')
        complete = re.search(r'LAB event=auto_complete corrections=(\d+) replays=(\d+)', text)
        trial20 = text.split('LAB event=trial trial=20 ', 1)[-1].split('LAB event=trial trial=21 ', 1)[0]
        corrections20 = len(re.findall('LAB event=cmc_correction', trial20))
        pawns = set(re.findall(r'LAB event=authority pawn=(\S+)', host))
        locked = re.findall(r'LAB event=authority pawn=\S+ speed=([\d.]+) max=([\d.]+) locked=1', host)
        evidence += [f'## {directory.name}', '',
            f'- Серверных Pawn в телеметрии: {len(pawns)}.',
            f'- Коррекций CMC всего: {complete[1]}; вызовов PrepMoveFor: {complete[2]}.',
            f'- Коррекций между trial=20 и trial=21 (намеренный запрет): {corrections20}.',
            f'- Событий server_reject на сервере: {host.count("LAB event=server_reject")}.',
            f'- Максимальная скорость в период серверного запрета по телеметрии: {max((float(x[0]) for x in locked), default=0):.1f} см/с.',
            f'- [Лог клиента](media/{args.run.name}/{directory.name}/client.log), [лог сервера](media/{args.run.name}/{directory.name}/host.log).', '']
    output = args.run.parent.parent / 'Results.md'
    output.write_text('\n'.join(lines + ['', 'Метрика: нажатие Shift → скорость >475 см/с; перед нажатием ходьба 450 см/с.',
        'Запланировано по 10 попыток каждого режима. Колонка n содержит число реальных response; отсутствующие замеры не подменяются числами.',
        'В CSV сохраняются также trial=20–22 (отказ и восстановление); они не входят в сравнительную таблицу.', '', *evidence]), encoding='utf-8')
    print(output)

if __name__ == '__main__':
    main()
