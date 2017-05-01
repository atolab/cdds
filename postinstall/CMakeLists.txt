

# The bitrock script has to be executed after all other installation steps have completed.
if(BITROCK_FOUND)
    set(INSTALL_BUILDER_CONF ${CMAKE_SOURCE_DIR}/postinstall/Bitrock.xml)
    file(TO_NATIVE_PATH "${INSTALL_BUILDER_CONF}" INSTALL_BUILDER_CONF_NATIVE)
    install(CODE
                "
                message(STATUS \"Executing Bitrock InstallBuilder\")
                message(STATUS \"Bitrock (build) (${INSTALL_BUILDER_CONF_NATIVE}) (${INSTALLER_PLATFORM})\")
                execute_process(COMMAND ${BITROCK_PATH} build ${INSTALL_BUILDER_CONF_NATIVE}
                                WORKING_DIRECTORY ${INSTALL_PATH})
                "
           )
endif()

