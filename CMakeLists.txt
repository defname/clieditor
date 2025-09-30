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

# --- Objekt-Bibliotheken erstellen ---
# Für jede Quelldatei eine eigene "OBJECT"-Bibliothek erstellen.
# Das gibt uns maximale Flexibilität beim Linken.
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
add_executable(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_OBJECTS})

add_compile_definitions(_XOPEN_SOURCE=700)



# --- Tools Ordner hinzufügen ---
add_subdirectory(tools)

# --- Test-Konfiguration ---
# CTest-Unterstützung aktivieren
enable_testing()

# Finde alle Testdateien
file(GLOB TEST_SOURCES "tests/test_*.c")

# Durch jede gefundene Testdatei loopen
foreach(TEST_SOURCE_FILE ${TEST_SOURCES})
    # Den Namen für das Executable aus dem Dateinamen ableiten
    # z.B. aus "tests/test_utf8.c" wird "test_utf8"
    get_filename_component(TEST_NAME ${TEST_SOURCE_FILE} NAME_WE)

    # Erstelle eine Liste aller Objekt-Bibliotheken, AUSSER der, die getestet wird.
    string(REPLACE "test_" "" ORIGINAL_SOURCE_NAME ${TEST_NAME})

    # Erstelle eine Liste der auszuschließenden Objekt-Bibliotheken.
    # Wir iterieren durch alle bekannten Sourcen und finden die, die wir ausschließen wollen.
    set(OBJECTS_TO_EXCLUDE "")
    foreach(SOURCE_FILE ${SOURCES})
        get_filename_component(FILENAME ${SOURCE_FILE} NAME_WE) # z.B. "main" oder "common/utf8"
        
        # Schließe immer main aus UND die Datei, die dem Testnamen entspricht.
        if(FILENAME STREQUAL "main" OR FILENAME STREQUAL ORIGINAL_SOURCE_NAME)
            string(REPLACE "/" "_" LIB_NAME_RAW ${SOURCE_FILE})
            string(REPLACE ".c" "" LIB_NAME_RAW ${LIB_NAME_RAW})
            list(APPEND OBJECTS_TO_EXCLUDE "obj_${LIB_NAME_RAW}")
        endif()
    endforeach()

    set(TEST_LINK_OBJECTS "")
    foreach(OBJ_LIB ${PROJECT_OBJECTS})
        list(FIND OBJECTS_TO_EXCLUDE ${OBJ_LIB} IS_EXCLUDED)
        if(IS_EXCLUDED EQUAL -1) # Wenn nicht in der Ausschlussliste gefunden...
            list(APPEND TEST_LINK_OBJECTS ${OBJ_LIB}) # ...dann zur Link-Liste hinzufügen.
        endif()
    endforeach()

    # Erstelle das Test-Executable und linke es gegen die gefilterten Objekt-Bibliotheken.
    add_executable(${TEST_NAME} ${TEST_SOURCE_FILE})
    target_link_libraries(${TEST_NAME} PRIVATE ${TEST_LINK_OBJECTS})

    # Den Test zu CTest hinzufügen, damit er mit `ctest` ausgeführt werden kann
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

endforeach()