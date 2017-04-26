add_library(GetTime INTERFACE)

include(CheckLibraryExists)

# First check whether libc has clock_gettime
check_library_exists(c clock_gettime "" HAVE_CLOCK_GETTIME_IN_C)

if(NOT HAVE_CLOCK_GETTIME_IN_C)
  # Before glibc 2.17, clock_gettime was in librt
  check_library_exists(rt clock_gettime "time.h" HAVE_CLOCK_GETTIME_IN_RT)
  if (HAVE_CLOCK_GETTIME_IN_RT)
    target_link_libraries(GetTime INTERFACE rt)
  endif()
endif()

