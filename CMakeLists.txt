cmake_minimum_required(VERSION 3.5)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
project(VortexDDS)

# Set install destination directory
if($ENV{DESTDIR})
    set(INSTALL_PATH $ENV{DESTDIR})
else()
    set(INSTALL_PATH "${CMAKE_BINARY_DIR}/install")
endif()

find_package(Abstraction QUIET REQUIRED)
find_package(CUnit QUIET)
find_package(Bitrock QUIET)

enable_testing()

include_directories(include)
add_subdirectory(src)
#add_subdirectory(vdds-server)
add_subdirectory(examples)
#add_subdirectory(rpc-examples)

#
# There's no such thing in cmake as a 'post-install' action. So, we have
# to do with a workaround. Cmake makes sure that all added sub directories
# are handled in sequence. This means that the last sub directory is always
# handled last.
#
# The postinstall directory must always be the last directory to make sure
# that the install steps within that directory are executed after all the
# other installation steps mentioned in the other sub-directories.
#
add_subdirectory(postinstall)