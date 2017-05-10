add_executable(UT_C99_EntityAPI "")
target_link_libraries(UT_C99_EntityAPI vdds cunit)

target_sources(UT_C99_EntityAPI PRIVATE "entity_api.c")

add_test(C99_EntityAPI UT_C99_EntityAPI)
