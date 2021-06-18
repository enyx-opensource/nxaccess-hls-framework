find_package(PythonInterp 3)
find_package(PythonLibs 3)
find_package(pybind11)


if (PythonInterp_FOUND AND PythonLibs_FOUND AND pybind11_FOUND)
    include_directories(${PYTHON_INCLUDE_PATH})
    include_directories(${CMAKE_SOURCE_DIR}/lib/include)

    set(PYBIND11_CPP_STANDARD -std=c++11)

    set(module_files
            module.cpp)

    pybind11_add_module(enyx_oe_hwstrat_hls_demo
        ${module_files})

    target_link_libraries(enyx_oe_hwstrat_hls_demo
        PRIVATE
        enyx-hw-c
        ${PYTHON_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT})

    install(
        TARGETS enyx_oe_hwstrat_hls_demo
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/enyx/oe/python/
        COMPONENT base
        )

endif()
