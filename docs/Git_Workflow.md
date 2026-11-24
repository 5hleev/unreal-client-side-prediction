# GitHub: ветка, задачи и Pull Request

По разрешению преподавателя для практической работы используется GitHub вместо GitVerse.
Репозиторий: [5hleev/unreal-client-side-prediction](https://github.com/5hleev/unreal-client-side-prediction).
Рабочая ветка `feature/prediction` создана от `main`; история разработки сохранена.

## Задачи

| Задача | Результат |
|---|---|
| [Двухпользовательский прототип от третьего лица](https://github.com/5hleev/unreal-client-side-prediction/issues/1) | Выполнено; закрывается при слиянии PR |
| [Предсказание спринта через сохранённые ходы CMC](https://github.com/5hleev/unreal-client-side-prediction/issues/2) | Выполнено; закрывается при слиянии PR |
| [Сравнение режима CSP с ожиданием ответа сервера](https://github.com/5hleev/unreal-client-side-prediction/issues/3) | Выполнено; закрывается при слиянии PR |
| [Серверный отказ, коррекция и восстановление спринта](https://github.com/5hleev/unreal-client-side-prediction/issues/4) | Выполнено; закрывается при слиянии PR |
| [Сетевые испытания и доказательства T1–T5](https://github.com/5hleev/unreal-client-side-prediction/issues/5) | Выполнено; закрывается при слиянии PR |
| [Документация, отчёт и переносимая Windows-сборка](https://github.com/5hleev/unreal-client-side-prediction/issues/6) | Выполнено; закрывается при слиянии PR |

## Pull Request

[PR #7: клиентское предсказание спринта и серверные коррекции](https://github.com/5hleev/unreal-client-side-prediction/pull/7)
из `feature/prediction` в `main` объединяет реализацию, испытания и документы.
Задачи #1–#6 связаны с PR и закрываются при его слиянии.
Независимое ревью другим человеком не проводилось; техническая самопроверка
и фактические ограничения описаны в [Verification.md](Verification.md).

## Проверка реализации

UE 5.8.4 / MSVC 14.44.35230: Editor/Game Development и BuildCookRun успешны.
Два процесса готового exe, Clean/Lag100/Loss175, реальные логи и нативные скриншоты.
При Lag100 медиана отклика: 241,72 мс без CSP и 16,67 мс с CSP.
При Loss175: 383,39 и 16,65 мс; 41 коррекция и 938 повторно проигранных ходов.
Сервер отклоняет запрещённый спринт и ограничивает скорость 450 см/с.
Данные и ограничения: [Results.md](Results.md), [матрица T1–T5](Prediction_Test_Matrix.md).

Исходники, отчёт и готовая Windows-сборка: [релиз v1.0.0](https://github.com/5hleev/unreal-client-side-prediction/releases/tag/v1.0.0).
В Git хранятся исходники, карта, документы, CSV, логи и скриншоты.
Кеши Unreal и готовая игра исключены через `.gitignore`; архивы доступны в релизе.
