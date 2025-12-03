# Установка зависимостей

## Проблема с правами доступа

При установке через Homebrew возникла проблема с правами доступа. Выполните следующие команды в терминале:

### Шаг 1: Исправить права доступа

```bash
sudo chown -R $(whoami) /usr/local/var/log
chmod u+w /usr/local/var/log
```

### Шаг 2: Установить зависимости

```bash
brew install cmake ninja googletest
```

## Альтернативные способы установки

### Вариант 1: Использовать Homebrew в /opt/homebrew (Apple Silicon)

Если у вас Mac с Apple Silicon, Homebrew может быть установлен в `/opt/homebrew`:

```bash
/opt/homebrew/bin/brew install cmake ninja googletest
```

### Вариант 2: Установить через MacPorts (если установлен)

```bash
sudo port install cmake ninja googletest
```

### Вариант 3: Скачать бинарники напрямую

- CMake: https://cmake.org/download/
- Ninja: https://github.com/ninja-build/ninja/releases
- Google Test: можно собрать из исходников или использовать vcpkg

### Вариант 4: Использовать Conda (если установлен)

```bash
conda install -c conda-forge cmake ninja
# Google Test нужно установить отдельно или собрать из исходников
```

## Проверка установки

После установки проверьте:

```bash
cmake --version
ninja --version
```

## После установки

Выполните сборку проекта:

```bash
cd /Users/user/Documents/driver-solution-cad
mkdir -p build && cd build
cmake -G Ninja ..
ninja
ctest --output-on-failure
```

