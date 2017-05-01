# Find the Bitrock InstallBuilder
#
# BITROCK_PATH    - the full path to the Bitrock InstallBuilder executable
# BITROCK_FOUND   - If false, don't attempt to use Bitrock Install Builder.
#
# This piece of code is inspired by https://github.com/BRAT-DEV/main/blob/master/CMakeModules/FindInstallBuilder.cmake

message(STATUS "Looking for Bitrock InstallBuilder")

# Main inspiration:
# https://github.com/BRAT-DEV/main/blob/master/CMakeLists.txt

if(WIN32)
    set(EXTRA_DIRLOCATION_TMP
        "C:/Program Files/Bitrock*"
        "C:/Program Files/Bitrock/*"
        "C:/Bitrock*"
        "C:/Bitrock/*"
        "D:/Program Files/Bitrock*"
        "D:/Program Files/Bitrock/*"
        "D:/Bitrock*"
        "D:/Bitrock/*"
        "E:/Program Files/Bitrock*"
        "E:/Program Files/Bitrock/*"
        "E:/Bitrock*"
        "E:/Bitrock/*"
    )

    file(GLOB EXTRA_DIRLOCATION_TMP ${EXTRA_DIRLOCATION_TMP})
    foreach (IBDIR_TMP ${EXTRA_DIRLOCATION_TMP})
        set(EXTRA_DIRS ${EXTRA_DIRS} "${IBDIR_TMP}/bin")
    endforeach(IBDIR_TMP)
endif()


if(APPLE)
    file(GLOB EXTRA_DIRLOCATION_TMP "/Applications/Bitrock*")
    foreach (IBDIR_TMP ${EXTRA_DIRLOCATION_TMP})
        set(EXTRA_DIRS ${EXTRA_DIRS} "${IBDIR_TMP}/bin/Builder.app/Contents/MacOS")
    endforeach(IBDIR_TMP)
    set(IB_EXECNAME installbuilder.sh)
elseif()
    set(IB_EXECNAME builder-cli)
else()
    set(IB_EXECNAME builder)
endif()

find_program(BITROCK_PATH
    NAMES ${IB_EXECNAME}
    PATHS
    ${INSTALL_BUILDER_DIR} ENV INSTALL_BUILDER_DIR
    ${EXTRA_DIRS}
    )

mark_as_advanced(
    BITROCK_PATH
    )

if(BITROCK_PATH)
    message(STATUS "Looking for Bitrock InstallBuilder - ${BITROCK_PATH}")
    set(BITROCK_FOUND "YES")
else(BITROCK_PATH)
    message(STATUS "Looking for Bitrock InstallBuilder - not found")
    set(BITROCK_FOUND "NO")
endif(BITROCK_PATH)
