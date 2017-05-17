cmake_minimum_required(VERSION 3.3) # For IN_LIST

get_property(languages GLOBAL PROPERTY ENABLED_LANGUAGES)
set(scan_build_supported Clang AppleClang GNU)

mark_as_advanced(
    languages
    scan_build_supported)

if("C" IN_LIST languages)
  include(CheckCCompilerFlag)

  if(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    set(ANALYZE_C_BUILD_FLAG "/analyze")
  elseif(CMAKE_C_COMPILER_ID IN_LIST scan_build_supported)
    message(STATUS "Static analysis for C using ${CMAKE_C_COMPILER_ID}-compiler is available by using 'scan-build' manually only")
  endif()

  if(DEFINED ANALYZE_C_BUILD_FLAG)
    CHECK_C_COMPILER_FLAG(${ANALYZE_C_BUILD_FLAG} C_COMPILER_HAS_ANALYZE_BUILD)

    if(C_COMPILER_HAS_ANALYZE_BUILD)
      if(CMAKE_GENERATOR MATCHES "Visual Studio")
        # $<COMPILE_LANGUAGE:...> may not be used with Visual Studio generators.
        add_compile_options(${ANALYZE_C_BUILD_FLAG})
      else()
        add_compile_options($<$<COMPILE_LANGUAGE:C>:${ANALYZE_C_BUILD_FLAG}>)
      endif()
    endif()
  endif()
endif()

if("CXX" IN_LIST languages)
  include(CheckCXXCompilerFlag)

  if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(ANALYZE_CXX_BUILD_FLAG "/analyze")
  elseif(CMAKE_C_COMPILER_ID IN_LIST scan_build_supported)
    message(STATUS "Static analysis for CXX using ${CMAKE_CXX_COMPILER_ID}compiler is available by using 'scan-build' manually only")
  endif()

  if(DEFINED ANALYZE_CXX_BUILD_FLAG)
    CHECK_CXX_COMPILER_FLAG(${ANALYZE_CXX_BUILD_FLAG} CXX_COMPILER_HAS_ANALYZE_BUILD)

    if(CXX_COMPILER_HAS_ANALYZE_BUILD)
      # /analyze was already added if generated target matches Visual Studio.
      if(NOT CMAKE_GENERATOR MATCHES "Visual Studio")
        add_compile_options($<$<COMPILE_LANGUAGE:CXX>:${ANALYZE_CXX_BUILD_FLAG}>)
      endif()
    endif()
  endif()
endif()

mark_as_advanced(
  ANALYZE_C_BUILD_FLAG
  C_COMPILER_HAS_ANALYZE_BUILD
  ANALYZE_CXX_BUILD_FLAG
  CXX_COMPILER_HAS_ANALYZE_BUILD)

