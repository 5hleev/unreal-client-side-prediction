# Доказательства реального запуска

Обязательные файлы после испытаний:

- `baseline_no_prediction.mp4` либо подписанная серия `baseline_no_prediction_*.png`.
- `prediction_enabled.mp4` либо `prediction_enabled_*.png`.
- `correction_example.mp4` либо `correction_example_*.png`.

В текущей поставке вместо видео используется серия PNG, разрешённая методичкой.
Версия UE 5.8.4, код aa8a730187ab1749aa3bdb549ec550d592becc2d.
Все кадры сняты FScreenshotRequest в настоящем standalone-клиенте.

| Файл | Содержание |
|---|---|
| baseline_no_prediction.png | Lag100, CSP OFF, отклик после RPC-ответа |
| prediction_enabled.png | Lag100, CSP ON, локальное ускорение до серверного ответа |
| correction_before.png | Loss175, trial=20: клиент ещё ожидает разрешённый спринт |
| correction_example.png | Loss175, trial=20: максимум 450, known denial=1, выросли счётчики CMC/replay |

Подробные подписи: [Evidence.md](../Evidence.md). Метаданные: session.txt.
Полные журналы и дополнительные кадры: run_20261123_195522/Clean, Lag100, Loss175.
portable_build.log содержит успешный BuildCookRun, portable_sha256.txt — SHA256 ZIP игры.
Нативные корневые PNG побайтово совпадают с кадрами из каталогов сеанса.
