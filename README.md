# 2D Physics Sandbox

Лабораторна робота №3 — «Використання зовнішніх бібліотек/фреймворків».
2D-пісочниця з фізикою на C++17: об'єкти падають, відскакують, їх можна хапати мишею
і кидати, а достатньо сильний удар руйнує їх на уламки.

## Збірка та запуск

Потрібні: компілятор з підтримкою C++17, CMake ≥ 3.21, git і доступ до інтернету

Запустіть Visual Studio та на початковому екрані виберіть «Відкрити локальну папку» (Open a local folder). 
Або скористайтеся пунктами меню: Файл → Відкрити → Папка... (File → Open → Folder...).
Виберіть папку, що містить файл CMakeLists.txt (наприклад, C:\dev\physics-sandbox), і натисніть «Вибрати папку» (Select Folder).

Зачекайте налаштування CMake

Натисніть Ctrl+F5 або клацніть зелену кнопку «Запуск».


## Керування

| Дія | Результат |
|---|---|
| `S` | створити вибраний у меню об'єкт під курсором |
| Кнопка **Spawn** | створити об'єкт у фіксованій точці над землею |
| Ліва кнопка миші | схопити об'єкт під курсором і тягнути; при відпусканні об'єкт зберігає швидкість руху миші (можна кидати). Кліки по меню ігноруються |
| `Space` | пауза / продовження |
| `C` | очистити сцену |
| `F5` / `F9` | зберегти / завантажити `scene.json` |

Меню справа: випадаючий список типу об'єкта, **Spawn**, слайдери гравітації, тертя та пружності,
кнопки Save / Load / Clear / Pause. Гравітація діє на весь світ одразу; тертя й пружність
застосовуються до **нових** об'єктів.

## Використані бібліотеки і що саме з них використано

| Бібліотека (версія) | Для чого | Використані можливості |
|---|---|---|
| **SFML 2.6.1** | вікно, малювання, введення | `sf::RenderWindow` (створення, `pollEvent`, `clear/display`, вертикальна синхронізація, відключення повтору клавіш, згладжування через `sf::ContextSettings`); події `Closed`, `LostFocus`, `KeyPressed`, `MouseButtonPressed/Released`; `sf::Mouse::getPosition`; `sf::Clock`, `sf::Time`; фігури `sf::CircleShape`, `sf::ConvexShape`, `sf::RectangleShape`, `sf::Color`. Аудіо та мережа вимкнені |
| **Box2D 2.4.1** (C++ API) | фізика | `b2World` (`Step`, `SetGravity`, `SetContactListener`, `QueryAABB`, `DestroyBody`); `b2Body`/`b2BodyDef` (статичні й динамічні тіла, початкова швидкість, user data); `b2PolygonShape` (`Set`, `SetAsBox`), `b2CircleShape`; `b2FixtureDef` (щільність, тертя, пружність); **`b2ContactListener::PostSolve` + `b2ContactImpulse`** для імпульсів зіткнень; `b2QueryCallback` + `b2Fixture::TestPoint` для вибору об'єкта під курсором; **`b2MouseJoint`** + `b2LinearStiffness` для перетягування. Не використовуються: інші види з'єднань, сенсори, raycast, debug draw |
| **Dear ImGui 1.89.9 + ImGui-SFML (гілка 2.6.x)** | меню | `ImGui::SFML::Init/ProcessEvent/Update/Render/Shutdown`; вікно (`Begin/End` із фіксованою позицією), `BeginCombo`/`Selectable`, `Button`, `SliderFloat`, `Text`/`TextWrapped`/`TextDisabled`, `PushStyleColor`, `Separator`; `WantCaptureMouse` і `WantCaptureKeyboard`, щоб світ ігнорував кліки по меню |
| **nlohmann/json 3.11.3** | збереження/завантаження сцени | побудова `json` з ініціалізаторів, `json::array`, `dump(2)`, `json::parse`, `at()`, `get<T>()`, `is_array()`, обробка `json::exception` |
| **spdlog 1.14.1** | логування | власний логер із двома приймачами (`stdout_color_sink_mt`, `basic_file_sink_mt`), шаблон повідомлення, рівні `info/warn/error/critical`, `flush_on` |

## Архітектура

Кожна бібліотека захована за власним класом; решта програми про неї не знає.

| Клас | Обгортає | Відповідальність |
|---|---|---|
| `PhysicsWorld` | Box2D | світ, тіла, слухач контактів, руйнування, перетягування, знімок сцени. У заголовку немає жодного типу Box2D (pimpl) |
| `Renderer` | SFML | вікно і малювання тіл, меж та уламків |
| `UiPanel` | ImGui / ImGui-SFML | бічне меню, повертає `UiActions` |
| `SceneSerializer` | nlohmann/json | `SceneData` ⇄ `scene.json`, перевірка вмісту |
| `initLogging()` | spdlog | налаштування логера |
| `Shape` → `BoxShape`, `CircleShape`, `TriangleShape`, `HeavyBallShape` | — | розмір, маса і міцність кожного типу; створюються через `ShapeFactory` |
| `DebrisEffect` | — | візуальні уламки (не тіла Box2D) |
| `VelocityTracker` | — | швидкість миші за останні ~0.1 с для кидка |
| `Application` | — | головний цикл, введення, зв'язок компонентів |

`Units.h` — єдине місце перерахунку пікселів у метри (50 px = 1 м) і перевороту осі Y
(початок світу — лівий нижній кут вікна). Власники ресурсів — `std::unique_ptr` /
`std::shared_ptr`; сирі вказівники на тіла Box2D неволодільні (власник — `b2World`).

## Як працює руйнування

1. Фізика крокує з фіксованим кроком 1/60 с (акумулятор у `Application::advanceSimulation`).
2. `BreakListener::PostSolve` сумує нормальні імпульси контакту. Якщо сума більша за
   `Shape::strength()` об'єкта, об'єкт лише **позначається** (знищувати тіла всередині
   колбека Box2D заборонено).
3. Після `world.Step()` `PhysicsWorld` знищує позначені тіла, фіксує `DestructionEvent`.
4. `DebrisEffect` перетворює подію на 3–4 уламки, які падають і зникають за 1–1.6 с.

| Об'єкт | Розмір | Маса | Міцність (Н·с) |
|---|---|---|---|
| Box | 1 × 1 м | 4 кг | 70 |
| Circle | радіус 0.5 м | 2 кг | 50 |
| Triangle | основа й висота 1.2 м | 1.5 кг | 30 |
| Heavy Ball | радіус 0.45 м | 30 кг | 1000 |

Значення підібрані так, щоб звичайне падіння не руйнувало об'єкти, а кинутий або
важкий предмет руйнував. Усі константи — у `Config.h` та `Shapes.h`.

## Структура репозиторію

```
CMakeLists.txt          збірка, FetchContent з фіксованими версіями
include/sandbox/*.h     заголовки
src/*.cpp               реалізація
README.md
retrospective_notes.md  нотатки для ретроспективи
```
## Використання ШІ

Ідею для лабораторної роботи надав LLM (Claude, Anthropic). 
Також ця LLM давала ідеї як реалізувати більшість функцій. 
LLM самостійно писала функціонал для системи координат, руйнування, перетягування та кидка і виправляла мої помилки в коді.
Документацію згенеровано за допомогою LLM (Claude, Anthropic)

## Джерела

- SFML: https://www.sfml-dev.org/documentation/2.6.1/
- Box2D 2.4: https://box2d.org/documentation/ (розділи про contact listener, mouse joint)
- Dear ImGui: https://github.com/ocornut/imgui ; ImGui-SFML: https://github.com/SFML/imgui-sfml
- nlohmann/json: https://json.nlohmann.me/
- spdlog: https://github.com/gabime/spdlog
