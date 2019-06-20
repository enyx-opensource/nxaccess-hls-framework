add_executable(enyx-oe-hwstrat-hls-demo
    HwStratHlsDemo.cpp)

target_include_directories(enyx-oe-hwstrat-hls-demo
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../lib/include
    PRIVATE ${CMAKE_SOURCE_DIR}/../../hw/project_nxaccess_hls/include
    PRIVATE ${CMAKE_SOURCE_DIR}/../../hw/project_nxaccess_hls/src
    )
target_link_libraries(enyx-oe-hwstrat-hls-demo
    enyx-oe-hwstrat-hls-engine)

install(TARGETS enyx-oe-hwstrat-hls-demo
        DESTINATION "${CMAKE_INSTALL_BINDIR}"
        COMPONENT base)