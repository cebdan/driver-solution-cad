# Установка ImGui + SDL2

## Требования

- CMake 3.20+
- C++20 компилятор
- SDL2
- OpenGL 3.3+

## Установка SDL2 на macOS

```bash
# Используя Homebrew
brew install sdl2

# Или скачать с https://www.libsdl.org/download-2.0.php
```

## Установка ImGui

ImGui - это header-only библиотека. Нужно скачать исходники:

```bash
cd /Users/user/Documents/driver-solution-cad
mkdir -p third_party
cd third_party
git clone https://github.com/ocornut/imgui.git
```

Или скачать вручную с https://github.com/ocornut/imgui/releases и распаковать в `third_party/imgui`

## Структура ImGui

После установки структура должна быть такой:

```
third_party/
└── imgui/
    ├── imgui.h
    ├── imgui.cpp
    ├── imgui_demo.cpp
    ├── imgui_draw.cpp
    ├── imgui_tables.cpp
    ├── imgui_widgets.cpp
    └── backends/
        ├── imgui_impl_sdl2.h
        ├── imgui_impl_sdl2.cpp
        ├── imgui_impl_opengl3.h
        └── imgui_impl_opengl3.cpp
```

## Сборка

```bash
cd build
cmake ..
make simple_gui_imgui
```

## Запуск

```bash
# macOS
open simple_gui_imgui.app

# Или напрямую
./simple_gui_imgui
```

## Преимущества ImGui + SDL2

1. **Полный контроль над окнами** - легко управлять кнопками окна на macOS
2. **Интеграция с OpenGL** - идеально для 3D рендеринга
3. **Легковесность** - нет тяжелых зависимостей
4. **Гибкость** - легко кастомизировать UI
5. **Производительность** - быстрый рендеринг

## Решение проблем

### SDL2 не найден

```bash
# Установить через Homebrew
brew install sdl2

# Или указать путь вручную в CMakeLists.txt
set(SDL2_INCLUDE_DIR "/opt/homebrew/include/SDL2")
set(SDL2_LIBRARY "/opt/homebrew/lib/libSDL2.dylib")
```

### ImGui не найден

Убедитесь, что ImGui находится в `third_party/imgui` относительно корня проекта.

### OpenGL ошибки

На macOS OpenGL включен автоматически. На Linux может потребоваться:

```bash
sudo apt-get install libgl1-mesa-dev
```


