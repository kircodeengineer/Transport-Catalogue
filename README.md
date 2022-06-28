# Transport-Catalogue
1. Название проекта

Транспортный справочник
2. Описание

Программа предназначена для:
- графического вывода схемы маршрутутов;
- поиска пути между автобусными остановками
- вычисления времени поездки
Исходный код программы состоит из следующих файлов
- transport_catalogue.cpp, transport_catalogue.h
Содержат объявление и определение класса, реализующего систему хранения автобусных маршрутов и обработки запросов к ней
- json.cpp, json.h 
...
4. Инструкция по развёртыванию
Поддерживаемые запросы ввода:
- BUS
- STOP

Поддерживаемые запросы вывода
- ROUTE
- BUS

...
Запросы к транспортному справочнику оформляются в виде json-файла с именем input.json и подаются на вход программы.
На выходе программа сформирует json-файл содержащий поля в соответствии с запросами ввода.



