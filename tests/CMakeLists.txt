# Finde alle Testdateien
file(GLOB TEST_SOURCES "test_*.c")

# Durch jede gefundene Testdatei loopen
foreach(TEST_SOURCE_FILE ${TEST_SOURCES})
    # Den Namen für das Executable aus dem Dateinamen ableiten
    # z.B. aus "tests/test_utf8.c" wird "test_utf8"
    get_filename_component(TEST_NAME ${TEST_SOURCE_FILE} NAME_WE)


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
    add_test(NAME ${TEST_NAME} COMMAND $<TARGET_FILE:${TEST_NAME}>)

endforeach()