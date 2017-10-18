find_package(Abstraction REQUIRED)

add_executable(ddsls ddsls.c)
target_link_libraries(ddsls vddsc OSAPI)

install(
  TARGETS ddsls
  DESTINATION "${CMAKE_INSTALL_BINDIR}"
  COMPONENT dev
)
