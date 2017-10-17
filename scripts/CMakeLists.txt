
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    configure_file(
       "vdds_install_examples.in" "vdds_install_examples" @ONLY)

    install(PROGRAMS ${CMAKE_BINARY_DIR}/scripts/vdds_install_examples
            DESTINATION "${CMAKE_INSTALL_BINDIR}"
            COMPONENT dev)
endif()

