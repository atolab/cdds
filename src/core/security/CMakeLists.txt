
PREPEND(headers_public_security "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/security>$<INSTALL_INTERFACE:include/security>"
    dds_security.h
)

#[[

target_include_directories(security
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>")


target_include_directories(security
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>")

target_sources(ddsc
    PUBLIC
        ${headers_public_security}
)

target_include_directories(security
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/src")]]


install(
  DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include/security"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  COMPONENT dev)


