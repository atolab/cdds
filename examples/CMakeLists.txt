if(NOT DEFINED CMAKE_INSTALL_EXAMPLESDIR)
    set(CMAKE_INSTALL_EXAMPLESDIR "examples" CACHE PATH "examples root dir")
endif()

add_subdirectory(helloworld)
add_subdirectory(helloquickworld)
add_subdirectory(helloinstanceworld)
add_subdirectory(roundtrip)
add_subdirectory(throughput)

install(
  DIRECTORY "${CMAKE_SOURCE_DIR}/examples/"
  DESTINATION "${CMAKE_INSTALL_EXAMPLESDIR}"
  COMPONENT dev
  PATTERN "CMakeLists.*" EXCLUDE)
