cmake_minimum_required(VERSION 3.3) # For IN_LIST

get_property(languages GLOBAL PROPERTY ENABLED_LANGUAGES)
set(scan_build_supported Clang AppleClang GNU)
if("C" IN_LIST languages)
  include(CheckCCompilerFlag)

  if(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    set(ANALYZE_C_BUILD_FLAG "/analyze")
  elseif(CMAKE_C_COMPILER_ID IN_LIST scan_build_supported)
    message(STATUS "ANALYZE_BUILD for C using ${CMAKE_C_COMPILER_ID}-compiler is available by using 'scan-build' manually only")
  endif()

  if(DEFINED ANALYZE_C_BUILD_FLAG)
    CHECK_C_COMPILER_FLAG(${ANALYZE_C_BUILD_FG} C_COMPILER_HAS_ANALYZE_BUILD)

    if(C_COMPILER_HAS_ANALYZE_BUILD)
      add_compile_options($<$<COMPILE_LANGUAGE:C>:${ANALYZE_C_BUILD_FLAG}>)
    endif()
  endif()
endif()

if("CXX" IN_LIST languages)
  include(CheckCXXCompilerFlag)

  if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(ANALYZE_CXX_BUILD_FLAG "/analyze")
  elseif(CMAKE_C_COMPILER_ID IN_LIST scan_build_supported)
    message(STATUS "ANALYZE_BUILD for CXX using ${CMAKE_CXX_COMPILER_ID}compiler is available by using 'scan-build' manually only")
  endif()

  if(DEFINED ANALYZE_CXX_BUILD_FLAG)
    CHECK_CXX_COMPILER_FLAG(${ANALYZE_CXX_BUILD_FLAG} CXX_COMPILER_HAS_ANALYZE_BUILD)

    if(CXX_COMPILER_HAS_ANALYZE_BUILD)
      add_compile_options($<$<COMPILE_LANGUAGE:CXX>:${ANALYZE_CXX_BUILD_FLAG}>)
    endif()
  endif()
endif()
