# Установка xtd для CAD System

## Быстрая установка

### Вариант 1: Использование Homebrew (рекомендуется для macOS)

```bash
brew install xtd
```

### Вариант 2: Сборка из исходников

1. **Установите зависимости:**
```bash
# На macOS
brew install cmake
# wxWidgets опционально, можно отключить
```

2. **Соберите xtd:**
```bash
cd /Users/user/Documents/xtd
mkdir -p build && cd build
cmake .. -DXTD_BUILD_TESTS=OFF -DXTD_BUILD_EXAMPLES=OFF -DXTD_BUILD_DOCUMENTATION=OFF
cmake --build . -j$(sysctl -n hw.ncpu)
sudo cmake --install .
```

3. **Если возникают проблемы с wxWidgets:**
```bash
# Отключите native drawing (будет использоваться системный рендерер)
cmake .. -DXTD_BUILD_DRAWING_NATIVE_WXWIDGETS=OFF
```

## Сборка GUI приложения

После установки xtd:

```bash
cd /Users/user/Documents/driver-solution-cad
mkdir -p build && cd build
cmake ..
cmake --build .
./simple_gui
```

## Альтернатива: Использование консольного меню

Если xtd не установлен, используйте консольную версию:

```bash
cd /Users/user/Documents/driver-solution-cad
./demo_menu_console
```


