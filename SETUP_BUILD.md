# Build Setup Instructions

## Prerequisites Installation

Для сборки проекта необходимо установить зависимости:

### macOS

```bash
# 1. Установить Xcode Command Line Tools (если не установлены)
xcode-select --install

# 2. Установить Homebrew (если не установлен)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 3. Установить зависимости
brew install cmake ninja googletest
```

### Проверка установки

```bash
cmake --version
ninja --version
```

## Сборка проекта

После установки зависимостей:

```bash
cd /Users/user/Documents/driver-solution-cad
mkdir -p build && cd build
cmake -G Ninja ..
ninja
```

## Запуск тестов

```bash
cd build
ctest --output-on-failure
```

Или напрямую:

```bash
./test_DependencyGraph
./test_Kernel
```

## Если тесты не собираются

Если Google Test не найден, можно собрать без тестов:

```bash
cmake -G Ninja -DBUILD_TESTS=OFF ..
ninja
```

