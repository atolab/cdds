
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    configure_file(
       "cmake/vdds_install_examples.in" "vdds_install_examples" @ONLY)

    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/vdds_install_examples
            DESTINATION "${CMAKE_INSTALL_BINDIR}"
            COMPONENT dev)
endif()

