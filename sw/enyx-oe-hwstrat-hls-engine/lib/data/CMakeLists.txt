configure_file(enyx-oe-hwstrat-hls-engine.pc.in
               ${CMAKE_CURRENT_BINARY_DIR}/enyx-oe-hwstrat-hls-engine.pc @ONLY)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/enyx-oe-hwstrat-hls-engine.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    COMPONENT devel)