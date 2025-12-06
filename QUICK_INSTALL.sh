#!/bin/bash

# Быстрая установка xtd и сборка GUI приложения
# Использование: ./QUICK_INSTALL.sh

set -e  # Остановка при ошибке

echo "╔══════════════════════════════════════════════════════════╗"
echo "║     Установка xtd и сборка GUI приложения CAD System     ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""

# Цвета для вывода
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Функция проверки команды
check_command() {
    if ! command -v $1 &> /dev/null; then
        echo -e "${RED}✗ $1 не найден${NC}"
        return 1
    else
        echo -e "${GREEN}✓ $1 установлен${NC}"
        return 0
    fi
}

# Проверка зависимостей
echo "📋 Проверка зависимостей..."
check_command brew || { echo "Установите Homebrew: /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""; exit 1; }
check_command cmake || { echo "Установите CMake: brew install cmake"; exit 1; }
check_command wx-config || { echo "Установите wxWidgets: brew install wxwidgets"; exit 1; }

echo ""
echo "🏗️  Шаг 1: Сборка xtd..."
cd /Users/user/Documents/xtd

if [ ! -d "build" ]; then
    mkdir -p build
fi

cd build

if [ ! -f "CMakeCache.txt" ]; then
    echo "Конфигурация CMake..."
    cmake .. \
        -DXTD_BUILD_TESTS=OFF \
        -DXTD_BUILD_EXAMPLES=OFF \
        -DXTD_BUILD_DOCUMENTATION=OFF
fi

echo "Сборка xtd (это может занять несколько минут)..."
cmake --build . -j$(sysctl -n hw.ncpu)

echo -e "${GREEN}✓ xtd собран${NC}"

echo ""
echo "📦 Шаг 2: Установка xtd (требуется пароль)..."
read -p "Установить xtd в систему? (y/n): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    sudo cmake --install .
    echo -e "${GREEN}✓ xtd установлен${NC}"
else
    echo -e "${YELLOW}⚠ Пропущена установка xtd. GUI приложение не будет собрано.${NC}"
    exit 0
fi

echo ""
echo "🎨 Шаг 3: Сборка GUI приложения..."
cd /Users/user/Documents/driver-solution-cad

if [ ! -d "build" ]; then
    mkdir -p build
fi

cd build

echo "Конфигурация CMake..."
cmake ..

echo "Сборка проекта..."
cmake --build . -j$(sysctl -n hw.ncpu)

if [ -f "simple_gui" ]; then
    echo -e "${GREEN}✓ GUI приложение собрано!${NC}"
    echo ""
    echo "🚀 Запуск GUI приложения..."
    echo ""
    ./simple_gui
else
    echo -e "${RED}✗ Ошибка сборки GUI приложения${NC}"
    exit 1
fi

