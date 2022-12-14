# Тестовое задание

## [Заметки](ANSWER.md)

Разработать консольное серверное приложение на языке **C++** работающее в среде ОС LINUX на пользовательском уровне. Приложение должно генерировать для клиента 3 последовательности, каждая из которых представляет собой целочисленный неотрицательный 64-х битный счётчик. Для каждой последовательности начальное значение и шаг между двумя соседними значениями задается клиентом (пользователем) произвольно.

Формат задания параметров - простой текст в `tcp/ip` сокет (для проверки используется telnet-клиент)

## Перечень команд

* `seqN x y` (где N - номер последовательности, x - начальное значение, y - шаг)
* `export seq` - экспорт последовательности в сокет

## Примечания
* Если в командах любой параметр не указан или указан как 0, программа не должна генерировать последовательность
* При переполнении счётчика подпоследовательность должна начинаться с начала (с начального значения x)
* Для удобства визуализации результата использовать текстовое представление сгенерированных чисел (для отображения в telnet-клиенте)
* Программа не должна аварийно завершаться при любых входных данных
* Для разработки использовать **только** стандартные возможности библиотеки C++ (11, 14, 17), проект должен собираться (make/cmake/...)
* Приложение должно быть многопоточным с использованием общего ресурса (единого хранилища настроек клиентов)
* С каждым пользователем ведётся обособленный диалог (пользователи не знают о действиях друг друга)
* По возможности использовать шаблоны проектирования
* Комментарии в коде приветствуются
* Прежде чем сдавать тестовое, проверьте и протестируйте у себя все пункты

## Примеры входных и выходных данных
Необходимо сгенерировать 3 следующие последовательности
Начальное значение 1, шаг 2: 1, 3, 5 ...
Начальное значение 2, шаг 3: 2, 5, 8 ...
Начальное значение 3, шаг 4: 3, 7, 11 ...
Команды:
```
seq1 1 2
seq2 2 3
seq3 3 4
export seq
```

Последняя команда передаёт в сокет последовательность:
```
1 2 3
3 5 7
5 8 11
...
```
