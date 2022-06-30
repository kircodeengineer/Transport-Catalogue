**1. Название проекта**

Транспортный справочник автобусных маршрутов

------------

**2. Описание**

Программа предназначена для:
1. графического вывода схемы маршрутутов;
2. поиска пути между автобусными остановками
3. вычисления времени поездки

Исходный код программы состоит из следующих файлов:
1. transport_catalogue.cpp, transport_catalogue.h
Содержат объявление и определение класса, реализующего систему хранения автобусных маршрутов и обработки запросов к ней
- json.cpp, json.h 
...

------------

**3. Инструкция по развёртыванию**

Запросы к транспортному справочнику оформляются в виде json-файла с именем input.json и подаются на вход программы.
На выходе программа сформирует json-файл содержащий поля в соответствии с запросами раздела stat_requests.
Файлы с примерами ввода (input.json) и вывода (result.json) программы находятся в папке example.
Запросы:
1)  base_requests - словарь запросов для заполнения транспортного справочника
а) запрос на добавление автобусного маршрута
- **is_roundtrip** - маршрут кольцевой или туда и обратно;
- **name** - номер маршрута;
- **stops** - массив имён остановок;
- **type** - тип запроса (**Bus**);

Пример
          
          {
              "is_roundtrip": true,
              "name": "297",
              "stops": [
                  "Stop1",  
                  "Stop2",  
                  "Stop3",  
              ],
              "type": "Bus"  
          }
          
б) запрос на добавление автобусной остановки
- **latitude** - широта;
- **longitude** - доглгота;
- **name** - название остановки;
- **road_distances** - дистанции от данного остановки до куазанной остановки;
- **type** - тип запроса (**Stop**);

Пример

          {
              "latitude": 55.55,
              "longitude": 37.77,
              "name": "Stop1",
              "road_distances": {
                  "Stop2": 2600,
                  "Stop3": 2200
              },
              "type": "Stop"
          }
2) render_settings - настройки выводимый карты маршрутов
- **bus_label_font_size** - размер шрифта номера автобусной остановки;
- **bus_label_offset** - смещение (тень номера автобусной остановки);
- **color_palette** - цветовая палитра для отображения линий маршрутов;
- **height** - высота изображения в пикселях;
- **line_width** - толщина линий, которыми рисуются автобусные маршруты;
- **padding** - отступ краёв карты от границ SVG-документа;
- **stop_label_font_size** -  размер текста, которым отображаются названия остановок;
- **stop_label_offset** -  смещение названия остановки относительно её координат на карте;
-**stop_radius** - радиус окружностей, которыми обозначаются остановки;
-**underlayer_color** - цвет подложки под названиями остановок и маршрутов;
-**underlayer_width** - толщина подложки под названиями остановок и маршрутов;

Пример

    "render_settings": {
              "bus_label_font_size": 20,
              "bus_label_offset": [
                  7,
                  15
              ],
              "color_palette": [
                  "green",
                  [
                      255,
                      160,
                      0
                  ],
                  "red"
              ],
              "height": 200,
              "line_width": 14,
              "padding": 30,
              "stop_label_font_size": 20,
              "stop_label_offset": [
                  7,
                  -3
              ],
              "stop_radius": 5,
              "underlayer_color": [
                  255,
                  255,
                  255,
                  0.85
              ],
              "underlayer_width": 3,
              "width": 200
          }

3) routing_settings - настройки для поиска пути
- **bus_velocity** - скорость автобуса;
- **bus_wait_time** - время ожидания автобуса;

Пример

      "routing_settings": {
          "bus_velocity": 30,
          "bus_wait_time": 2
      }
4)  stat_requests  - блок запросов к транспортному справочнику
а) Вывод информации о маршруте
- **id** - id запроса;
- **name** - номер маршрута
- **type** - тип запроса (**Bus**);

Пример

          {
              "id": 1,
              "name": "125",
              "type": "Bus"
          }
б) Вывод всех маршрутах проходящих через остановку
- **id** - id запроса;
- **name** - название остановки
- **type** - тип запроса (**Stop**);

Проимер

          {
              "id": 4,
              "name": "Stop25",
              "type": "Stop"
          }
в) Вывод пути между остановками
- **from** - название остановки отправления;
- **id** - id запроса;
- **to** - название остановки прибытия;
- **type** - тип запроса (**Route**);

Пример

          {
              "from": "Stop1",
              "id": 5,
              "to": "Stop45",
              "type": "Route"
          }
          
Системные требования:
Компилятор C++17 (и выше).

------------

**4. Планы по доработке**
1. Удалене комментариев, связанных с постановкой задачи;
2. Добавление недостающих комментариев для автоматического документирование;
3. Разбиение больших функций на подфункции;
4. Приведение code-style к общему знаменателюж
5. Реализовать вывод SVG в отдельный файл
6. Продолжить описание ихсодных файлов в README.md
