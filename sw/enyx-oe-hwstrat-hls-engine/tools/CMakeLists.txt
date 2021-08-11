cmake_minimum_required (VERSION 3.5.1)

project (enyx-oe-hwstrat-hls-tools CXX C)

macro(create_tool name source_file)
    add_executable(enyx-oe-hwstrat-hls-${name}
        ${source_file})
    target_include_directories(enyx-oe-hwstrat-hls-${name}
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../lib/include)
    target_link_libraries(enyx-oe-hwstrat-hls-${name}
        enyxutils enyx-hw-c)

    install(TARGETS enyx-oe-hwstrat-hls-${name}
    DESTINATION "${CMAKE_INSTALL_BINDIR}"
    COMPONENT base)
endmacro()

create_tool(nxbus-injector nxbus_injector.cpp)
create_tool(hwstrat-conf-injector hwstrat_conf_injector.cpp)
create_tool(trigger-reader trigger_reader.cpp)
create_tool(hwstrat-conf-reader hwstrat_conf_reader.cpp)
