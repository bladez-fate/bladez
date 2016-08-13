# RTS Contest TODO
Must have features:
- hit points indicators
- resource(ore&oil) and supply indicators
- auto resource mining from mine/pumpjack
- unit group hotkeys 1-9 (save/select/show/add)
- select all units button/key
- unit weapon cooldown
- unit commands AI (move, attack, hold)
- unit "hold mode" should allow friendly units to pass through
- computer player AI
- neutral units AI
- neutral/enemy building capture if only ur unit are close within N seconds period
- building selection
- unit production
- unit production control (stop/repeat/once/choose unit)
- unit production indicator (progress bar)
- rally points
- unit/building control panel (with button for commands)
- unit/building selection panel (with selected units list)
- minimap panel
- popup tip with hotkey and description
- description texts
- galaxy generator with adjustable seed
- main menu scene (simplest)
- rocket launcher (unit)
- rocket (projectile) with area damage/impulse effect
- jeep (unit)
- bullet (projectile); require special physics handling
- unit movement animation (rolling wheels)
- unit destruction animation with destruction parts (physics objs with limited lifetime)
- building functioning animation (while producing resources/units)
- unit shoot animation
- factory upgrade (to produce another unit type, max 2 unit types per factory)
- upgraded factory model
- factory upgrading animation
- victory condition (no enemy buildings)
- victory scene
- sound & music

Additional features:
- destructible terrain?
- background mountains (just for beauty)
- clouds and rain (rain affect friction)
- sun with day/night on planet
- mountains with different texture & material (ice)
- thunder and lighting
- background volcanos, periodically erupting lava that damage units

# TODO
- Продумать систему ресурсов и их транспорта
- Юниты
  - Загружаемые модели стандартного формата
  - Выделение
  - Управление
  - Грузы, передача груза и группы юнитов не требующие передачи грузов
  - AI для выполнения сложных приказов
- Разные локальные PhysicsWorld для удалённые астрономических объектов и один общий глодальный PhysicalWorld для движения локальных миров
- Детализация планет
  - Изменяемый ландшафт (как в scorch)
  - Горы
  - Вода
  - Ядро
  - Вулканическая активность
  - Приливные взаимодействия
- Звёзды
- Генератор мира
  - Генератор планет
  - Генератор газовых гигантов
  - Генератор спутников
  - Генератор астероидных поясов
  - Генератор звёздных систем
  - Генератор галактики
- Сохранение/загрузка игры
- StartScene перед запуском игры
- Миникарта
- Продумать дерево развития

# Технологии
- рудники
- шахты
- лесопилка
- корабли
- сталь
- порох -> динамит
- металлургия -> электричество -> радио
- паровой двигатель
- поезда

# Окружение и материалы
- земля (основная поверхность на равнинах)
- дерево (лес, сооружения)
- камень (горы, сооружения)
- железо (ядра планет земной группы, сооружения)
- вода
- железная дорога

# Как бы это назвать?
- лестницы
- тоннели
- мосты
- подземелья
- веревки и тросы

# Здания
- дом/школа/институт/... -> рабочий
- дом/казарма/академия -> воин
- лесопилка -> лук, стрелы, деревянная телега
- корабельная верфь -> лодка, корабль
- кузница -> топор, кирка, меч, щит, железная телега
- шахта -> создаёт вход в подземелье, который могут игнорировать проходящие сверху

# Каркасы (hull)
В каркас может помещатся команда (для начала один человек) военных или гражданских
в зависимости от этого каркас может выполнять гражданские или военные функции
(транспортные, строительные, добыча, ремонт, военные действия)
Каркасы могут содержать другие каркасы (транспортировка или управление)
- рабочий
- воин
- корабль
- телега (cart) (для катапульты, баллисты и перевозок)
- поезд и вагоны
- автомобиль
- танк

# Оружие
- топор
- кирка
- меч
- лук/стрелы (масло)
- катапульта/снаряды (камни, масло)
- баллиста)

# Броня, защита и т.п.
- щит
- стены
- башни
- ров и мост

# Ресурсы
- вода
- дерево
- камень
- железная руда
- уголь
- нефть
- газ
- урановая руда
