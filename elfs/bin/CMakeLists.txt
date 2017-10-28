cmake_minimum_required (VERSION 3.2)

# Setup project name
project (bin)

file(GLOB SOURCES "src/*cpp") 
foreach (SRC ${SOURCES})
        get_filename_component(NAME ${SRC} NAME_WE)
        add_executable(${NAME} ${SRC})
        target_link_libraries(${NAME} PUBLIC "ustd")
        get_filename_component(BIN "./../../p1" ABSOLUTE)
        install(FILES ${NAME} DESTINATION ${BIN})
endforeach()