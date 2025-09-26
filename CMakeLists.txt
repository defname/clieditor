cmake_minimum_required(VERSION 3.10)

# Projektname dynamisch aus dem Ordnernamen ableiten
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME} C)
set(CMAKE_C_STANDARD 99)

# Compiler-Warnungen aktivieren (für GCC/Clang)
if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Werror)
endif()

# Alle C-Dateien im Spielsrc sammeln
file(GLOB_RECURSE SOURCES "src/*.c")

# Executable erstellen
add_executable(${PROJECT_NAME} ${SOURCES})

# commons-Bibliothek verlinken
target_link_libraries(${PROJECT_NAME})

# Raylib + Mathebibliothek verlinken
find_package(raylib 4.5 QUIET)
if(raylib_FOUND)
    target_link_libraries(${PROJECT_NAME})
endif()
