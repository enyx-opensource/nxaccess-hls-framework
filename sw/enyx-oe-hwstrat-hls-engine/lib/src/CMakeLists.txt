add_library(enyx-oe-hwstrat-hls-engine SHARED
        AlgorithmDriver.cpp
        ErrorCode.cpp
        Protocol.cpp)

target_include_directories(enyx-oe-hwstrat-hls-engine
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

target_link_libraries(enyx-oe-hwstrat-hls-engine
        ${EnyxLibs_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        ${Rt_LIBRARY})

set_target_properties(enyx-oe-hwstrat-hls-engine
        PROPERTIES
            OUTPUT_NAME enyx-oe-hwstrat-hls-engine${CPACK_PACKAGE_VERSION_MAJOR}
            VERSION "${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}"
            SOVERSION ${CPACK_PACKAGE_VERSION_MAJOR})

install(TARGETS enyx-oe-hwstrat-hls-engine
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT base)