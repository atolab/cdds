set(CMAKE_INSTALL_EXAMPLESDIR "${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}/examples")

add_subdirectory(helloworld)
add_subdirectory(helloquickworld)
add_subdirectory(helloinstanceworld)
add_subdirectory(roundtrip)
add_subdirectory(throughput)

install(
  DIRECTORY "${CMAKE_SOURCE_DIR}/examples/"
  DESTINATION "${CMAKE_INSTALL_EXAMPLESDIR}"
  COMPONENT dev
  PATTERN "CMakeLists.export" EXCLUDE
  PATTERN "examples/CMakeLists.txt" EXCLUDE)
