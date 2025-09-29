# tools/CMakeLists.txt

# alle .c-Dateien in diesem Ordner sammeln
file(GLOB TOOL_SOURCES *.c)

foreach(tool_src ${TOOL_SOURCES})
    # Dateiname ohne Pfad und Suffix extrahieren
    get_filename_component(tool_name ${tool_src} NAME_WE)

    # Executable für jede .c-Datei bauen
    add_executable(${tool_name} ${tool_src})

    # falls dein Code aus src/ mitgenutzt werden soll:
    target_include_directories(${tool_name} PRIVATE ${CMAKE_SOURCE_DIR}/src)
    # target_link_libraries(${tool_name} PRIVATE ) # falls du libs hast
endforeach()
