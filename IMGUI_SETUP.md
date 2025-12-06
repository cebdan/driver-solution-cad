# Установка и использование Dear ImGui

## Шаг 1: Установка ImGui

### Вариант A: Git Submodule (рекомендуется)

```bash
cd /Users/user/Documents/driver-solution-cad

# Добавить ImGui как submodule
git submodule add https://github.com/ocornut/imgui.git third_party/imgui

# Инициализировать submodule
git submodule update --init --recursive
```

### Вариант B: Прямая загрузка

```bash
cd /Users/user/Documents/driver-solution-cad/third_party

# Скачать ImGui
git clone https://github.com/ocornut/imgui.git

# Или скачать ZIP с GitHub и распаковать в third_party/imgui/
```

## Шаг 2: Обновление CMakeLists.txt

Добавьте ImGui в `CMakeLists.txt`:

```cmake
# После секции с GLFW, перед созданием CADUI
if(GLFW_FOUND)
    # ImGui
    set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/third_party/imgui)
    
    # ImGui sources
    set(IMGUI_SOURCES
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
    )
    
    # ImGui backends for GLFW + OpenGL
    set(IMGUI_BACKENDS
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl2.cpp
    )
    
    # Create ImGui library
    add_library(imgui STATIC
        ${IMGUI_SOURCES}
        ${IMGUI_BACKENDS}
    )
    
    target_include_directories(imgui PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
        ${GLFW_INCLUDE_DIR}
    )
    
    if(DEFINED GLFW_TARGET)
        target_link_libraries(imgui PUBLIC ${GLFW_TARGET})
    else()
        target_link_libraries(imgui PUBLIC ${GLFW_LIBRARY})
    endif()
    
    target_link_libraries(imgui PUBLIC OpenGL::GL)
    
    # macOS frameworks
    if(APPLE)
        target_link_libraries(imgui PUBLIC
            ${COCOA_FRAMEWORK}
            ${IOKIT_FRAMEWORK}
            ${COREVIDEO_FRAMEWORK}
        )
    endif()
    
    # Create UI library (existing code)
    add_library(CADUI STATIC
        src/UI/Window.cpp
        src/UI/WindowConfig.cpp
        src/UI/Application.cpp
        src/UI/RenderCache.cpp
        src/UI/Octree.cpp
    )
    
    # ... остальной код CADUI ...
    
    # Link ImGui to CADUI
    target_link_libraries(CADUI PUBLIC imgui)
```

## Шаг 3: Интеграция в Application

### 3.1 Добавить заголовки в `include/UI/Application.h`

```cpp
// В начале файла
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"
```

### 3.2 Инициализация в `Application::Application()`

```cpp
Application::Application()
    : ... {
    // ... существующий код ...
    
    // Инициализация ImGui для всех окон
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Стиль ImGui
    ImGui::StyleColorsDark();
    
    // Инициализация для каждого окна
    ImGui_ImplGlfw_InitForOpenGL(windowTopBar_->getHandle(), true);
    ImGui_ImplOpenGL2_Init();
    
    // ... остальной код ...
}
```

### 3.3 Очистка в деструкторе

```cpp
Application::~Application() {
    // ... существующий код ...
    
    // Очистка ImGui
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
```

### 3.4 Рендеринг в `Application::run()`

```cpp
void Application::run() {
    while (true) {
        // ... существующий код ...
        
        // Для каждого окна с ImGui
        if (windowTopBar_ && !windowTopBar_->shouldClose()) {
            glfwMakeContextCurrent(windowTopBar_->getHandle());
            
            // Начать новый кадр ImGui
            ImGui_ImplOpenGL2_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            
            // Ваш UI код
            renderTopBar(windowTopBar_.get());
            
            // ImGui окна
            if (showUIEditor_) {
                renderUIEditor();
            }
            
            // Рендеринг ImGui
            ImGui::Render();
            ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
            
            windowTopBar_->swapBuffers();
        }
        
        // ... остальной код ...
    }
}
```

## Шаг 4: Создание UI Editor

Добавьте метод в `Application`:

```cpp
void Application::renderUIEditor() {
    ImGui::Begin("UI Editor", &showUIEditor_);
    
    // Список окон
    if (ImGui::CollapsingHeader("Windows")) {
        if (ImGui::Button("Top Bar")) {
            selectedWindow_ = "TopBar";
        }
        if (ImGui::Button("Left Tools")) {
            selectedWindow_ = "LeftTools";
        }
        if (ImGui::Button("Main View")) {
            selectedWindow_ = "MainView";
        }
        if (ImGui::Button("Right Panel")) {
            selectedWindow_ = "RightPanel";
        }
    }
    
    // Редактирование выбранного окна
    if (!selectedWindow_.empty()) {
        ImGui::Separator();
        ImGui::Text("Editing: %s", selectedWindow_.c_str());
        
        WindowSettings settings = loadWindowSettings(selectedWindow_);
        
        int x = settings.x;
        int y = settings.y;
        int w = settings.width;
        int h = settings.height;
        
        if (ImGui::InputInt("X", &x)) {
            settings.x = x;
            saveWindowSettings(selectedWindow_, settings);
        }
        if (ImGui::InputInt("Y", &y)) {
            settings.y = y;
            saveWindowSettings(selectedWindow_, settings);
        }
        if (ImGui::InputInt("Width", &w)) {
            settings.width = w;
            saveWindowSettings(selectedWindow_, settings);
        }
        if (ImGui::InputInt("Height", &h)) {
            settings.height = h;
            saveWindowSettings(selectedWindow_, settings);
        }
    }
    
    // Редактирование текста
    if (ImGui::CollapsingHeader("Text Editor")) {
        static char textBuffer[1024] = "";
        ImGui::InputTextMultiline("Edit Text", textBuffer, sizeof(textBuffer));
        if (ImGui::Button("Apply")) {
            // Сохранить текст в конфигурацию
        }
    }
    
    ImGui::End();
}
```

## Шаг 5: Добавить переменные состояния

В `include/UI/Application.h`:

```cpp
private:
    // UI Editor state
    bool showUIEditor_ = false;
    std::string selectedWindow_;
```

## Шаг 6: Горячая клавиша для открытия редактора

В `setupCallbacks()`:

```cpp
glfwSetKeyCallback(windowTopBar_->getHandle(), [](GLFWwindow* window, int key, int scancode, int action, int mods) {
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app && key == GLFW_KEY_F12 && action == GLFW_PRESS) {
        app->showUIEditor_ = !app->showUIEditor_;
    }
});
```

## Шаг 7: Сборка

```bash
cd build
cmake -G Ninja ..
ninja
```

## Использование

1. Запустите приложение: `./bin/driver-solution-cad`
2. Нажмите **F12** для открытия UI Editor
3. Выберите окно для редактирования
4. Измените позицию, размер, текст
5. Изменения сохраняются в `close.config`

## Дополнительные инструменты

### ImGuiBuilder (отдельное приложение)

```bash
# Клонировать
git clone https://github.com/dfranx/ImGuiBuilder.git
cd ImGuiBuilder
mkdir build && cd build
cmake ..
make

# Запустить
./ImGuiBuilder
```

### ImGuiStudio

```bash
git clone https://github.com/patrickcjk/imgui_studio.git
# Следуйте инструкциям в README
```

## Полезные ссылки

- [Dear ImGui Documentation](https://github.com/ocornut/imgui)
- [ImGui Examples](https://github.com/ocornut/imgui/tree/master/examples)
- [ImGui Wiki](https://github.com/ocornut/imgui/wiki)

