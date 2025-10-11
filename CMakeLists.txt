cmake_minimum_required(VERSION 3.10)

# Projektname dynamisch aus dem Ordnernamen ableiten
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME} C)
set(CMAKE_C_STANDARD 99)

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Debug' as none was specified.")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: Debug Release." FORCE)
endif()

# Compiler warnings and build-type specific flags (for GCC/Clang)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(STATUS "Debug build: Enabling sanitizers.")
        add_compile_options(-fsanitize=address,leak,undefined -fno-omit-frame-pointer -g)
        add_link_options(-fsanitize=address,leak,undefined)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        message(STATUS "Release build: Enabling optimizations.")
        add_compile_options(-O3 -DNDEBUG)
    endif()
endif()

# Alle C-Dateien im Spielsrc sammeln
file(GLOB_RECURSE SOURCES "src/*.c")
list(FILTER SOURCES EXCLUDE REGEX "src/main\\.c$")

# --- Objekt-Bibliotheken erstellen ---
# Für jede Quelldatei eine eigene "OBJECT"-Bibliothek erstellen.
set(PROJECT_OBJECTS "")
foreach(SOURCE_FILE ${SOURCES})
    # Erstelle einen Namen relativ zum Projektverzeichnis, z.B. "src/core/widget.c"
    # Erstelle einen eindeutigen Namen für die Objekt-Bibliothek, z.B. aus "src/core/widget.c" wird "obj_core_widget"
    string(REPLACE "/" "_" LIB_NAME_RAW ${SOURCE_FILE})
    string(REPLACE ".c" "" LIB_NAME_RAW ${LIB_NAME_RAW})
    set(LIB_NAME "obj_${LIB_NAME_RAW}")

    add_library(${LIB_NAME} OBJECT ${SOURCE_FILE})
    # Wichtig: Jede Objekt-Bibliothek braucht die richtigen Include-Pfade.
    # PUBLIC sorgt dafür, dass der Pfad sowohl für die Kompilierung des Objekts selbst als auch für alle Ziele, die es verwenden, gilt.
    target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

    list(APPEND PROJECT_OBJECTS ${LIB_NAME})
endforeach()

# --- Haupt-Executable erstellen ---
add_executable(${PROJECT_NAME} src/main.c)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_OBJECTS})

add_compile_definitions(_XOPEN_SOURCE=700)



# --- Add tools ---
add_subdirectory(tools)

# --- Add tests ---
add_subdirectory(tests)