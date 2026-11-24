# Практическая работа №4 — Client-Side Prediction в Unreal Engine

**Шлеев Максим Евгеньевич, ИКБО-21-23.**

PredictionLab — C++ проект вида от третьего лица: ACharacter, камера на SpringArm,
ходьба, прыжок и спринт 450 → 900 см/с. Спринт встроен в сохранённые ходы CMC.
Два режима переключаются F2; F3 создаёт серверный запрет для демонстрации коррекции.
Ходьба в обоих режимах использует штатное предсказание CMC.

**Состояние:** проект собран на **UE 5.8.4 / MSVC 14.44.35230**. Карта создана,
самостоятельная Windows-версия упакована. Реальные результаты двухпроцессных испытаний
и подписанные скриншоты: [Results.md](docs/Results.md).
По согласованию с преподавателем работа опубликована на GitHub вместо GitVerse:
[репозиторий](https://github.com/5hleev/unreal-client-side-prediction),
[задачи и Pull Request](docs/Git_Workflow.md).

## Сдача на ноутбуке без UE

Скачать **PredictionLab_Windows.zip** из [релиза v1.0.0](https://github.com/5hleev/unreal-client-side-prediction/releases/tag/v1.0.0)
и распаковать. В том же релизе доступны архив исходников `Prac_4.zip` и отчёт PDF.
Контрольная сумма готовой игры: [SHA256](docs/media/portable_sha256.txt).

Перенести содержимое `dist/Windows` или распаковать `dist/PredictionLab_Windows.zip`.
Запустить `Start_Clean.bat`, затем `Start_Lag100.bat` и `Start_Loss175.bat`.
Перед сменой профиля закрыть оба окна. Подробности: [инструкция для ноутбука](docs/Portable_Guide.md).
`Prac_4.zip` содержит исходники/документы; `PredictionLab_Windows.zip` — готовую игру.

## Запуск на Windows

Для сборки нужны Unreal Engine 5.8 и Visual Studio 2022 / Build Tools с C++,
MSVC и Windows SDK. Указать **корень установленного движка**, содержащий папку Engine:

```powershell
git clone https://github.com/5hleev/unreal-client-side-prediction.git prac_4
cd prac_4
powershell -ExecutionPolicy Bypass -File tools/build.ps1 -EngineRoot 'F:\Epic Games\UE_5.8'
powershell -ExecutionPolicy Bypass -File tools/play.ps1 -EngineRoot 'F:\Epic Games\UE_5.8' -Profile Lag100
```

Первый скрипт собирает Editor target и создаёт `Content/Maps/PredictionArena.umap`
через встроенный Python Unreal. Второй запускает два видимых окна: listen server и клиент.
Если клиент стартовал раньше готовности сервера, в клиентской консоли: `open 127.0.0.1:7777`.
Путь `F:\Epic Games\UE_5.8` соответствует установленному движку UE 5.8.4.
На текущем компьютере кеши Intermediate/Saved и результат dist размещены через
ссылки на F:/UE_Prac4_Build, чтобы не заполнять диск C:.

Сначала выполните профиль `Clean`, затем `Lag100` и `Loss175`.
Измеряйте отклик **в окне клиента**, не хоста. Закройте оба окна перед следующим профилем.
Для PIE: откройте uproject и карту PredictionArena; Number of Players = 2,
Net Mode = Play As Listen Server, New Editor Window. `Play As Client` создаёт отдельный
сервер, поэтому это другая топология. Для воспроизводимого опыта предпочтителен play.ps1.

## Управление

| Клавиша | Действие |
|---|---|
| WASD / мышь | Перемещение / камера |
| Space | Прыжок |
| Left Shift | Спринт, удерживать на земле |
| F2 | CSP спринта ON/OFF; сначала отпустить Shift и дождаться скорости 450 |
| F3 | Учебный серверный запрет спринта; повторное нажатие снимает запрет |
| ~ | Консоль Unreal |

HUD показывает сетевую роль, режим, скорость, намерение и время реакции.
Геометрический персонаж и арена не требуют Starter Content или скачивания ассетов.
Это самостоятельная C++ реализация устройства Third Person; сгенерированный Epic
шаблон Third Person и его анимационные ассеты в поставку не включены.

## Материалы

- [Реализация, поток данных, серверные проверки](docs/Prediction_Implementation.md).
- [Матрица испытаний T1–T5](docs/Prediction_Test_Matrix.md).
- [Подробный сценарий запуска и защиты](docs/Launch_and_Defense.md).
- [Теория и ответы](docs/Defense.md).
- [Задачи и текст Pull Request](docs/Git_Workflow.md).
- [Правила записи доказательств](docs/media/README.md).
- [Фактические результаты](docs/Results.md) и [статус проверки](docs/Verification.md).

Логи каждого запуска: `Saved/Runs/<дата>_<профиль>/`. Извлечение фактических замеров:

```powershell
python tools/analyze_logs.py Saved/Runs/<сеанс>/client.log --output docs/measurements/Lag100.csv
```

Скрипт не создаёт данные без реальных событий. Папки ПР1–ПР3 не изменяются.

Отчёт: `docs/ПР4_Шлеев_ИКБО-21-23.docx` и одноимённый PDF.
Обновить: `python tools/export_report.py` (нужны python-docx и pymupdf).
Архив для передачи: `Prac_4.zip`; обновить `python tools/package_submission.py`.
В архив входят исходники и документы; карта включается, если уже создана в Unreal.
Кеши сборки и локальная Git-история в архив не включаются.

Повторить автоматический опыт на готовой игре (23 попытки, около 95 секунд на профиль):

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools/run_network_tests.ps1 -Render -Portable
python tools/summarize_experiment.py docs/media/<новый_сеанс>
```

В итоговое сравнение берутся первые 10 попыток каждого режима; последние три —
намеренный отказ и проверка восстановления. F3 доступен только как учебный инструмент.
