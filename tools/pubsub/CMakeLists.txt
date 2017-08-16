add_executable(pubsub pubsub.c common.c testtype.c porting.c)
target_link_libraries(pubsub util VortexDDS::vddsc)

if(WIN32)
  # Many of the secure versions provided by Microsoft have failure modes
  # which are not supported by our abstraction layer, so efforts trying
  # to use the _s versions aren't typically the proper solution and C11
  # (which contains most of the secure versions) is 'too new'. So we rely
  # on static detection of misuse instead of runtime detection, so all
  # these warnings can be disabled on Windows.
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
 endif()
