# Быстрый старт CAD System

## ✅ Что готово:

1. **GUI приложение** (`examples/simple_gui.cpp`) - полноценное окно с меню
2. **Консольное меню** (`demo_menu_console`) - работает без установки
3. **Все классы** - Solution, Document2D, CS, Point2D, Point3D и др.

## 🚀 Запуск консольной версии (работает сейчас):

```bash
cd /Users/user/Documents/driver-solution-cad
./demo_menu_console
```

## 🖥️ Запуск GUI версии (требует xtd):

### Вариант 1: Установка через Homebrew
```bash
brew install wxwidgets
brew install xtd
```

### Вариант 2: Сборка xtd из исходников
```bash
# Установите wxWidgets
brew install wxwidgets

# Соберите xtd
cd /Users/user/Documents/xtd
mkdir -p build && cd build
cmake .. -DXTD_BUILD_TESTS=OFF -DXTD_BUILD_EXAMPLES=OFF
cmake --build . -j$(sysctl -n hw.ncpu)
sudo cmake --install .
```

### Сборка GUI приложения
```bash
cd /Users/user/Documents/driver-solution-cad
mkdir -p build && cd build
cmake ..
cmake --build .
./simple_gui
```

## 📋 Меню GUI приложения:

- **File** → New 2D Document (Ctrl+N) ← Создать 2D документ
- **File** → Open/Save/Save As
- **Edit** → Undo/Redo/Cut/Copy/Paste  
- **View** → Zoom In/Out/Fit to Window
- **Tools** → Options/Terminal
- **Help** → About/Documentation

