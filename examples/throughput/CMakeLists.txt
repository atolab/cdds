cmake_minimum_required(VERSION 3.5)

if (NOT TARGET VortexDDS::vddsc)
    # Find the VortexDDS package. If it is not in a default location, try
    # finding it relative to the example where it most likely resides.
    find_package(VortexDDS REQUIRED PATHS "${CMAKE_SOURCE_DIR}/../../")
endif()

# This is a convenience function, provided by the VortexDDS package,
# that will supply a library target related the the given idl file.
# In short, it takes the idl file, generates the source files with
# the proper data types and compiles them into a library.
idlc_generate(Throughput_lib Throughput.idl)

# Both executables have only one related source file.
add_executable(ThroughputPublisher publisher.c)
add_executable(ThroughputSubscriber subscriber.c)

# Both executables need to be linked to the idl data type library and
# the vddsc API library.
target_link_libraries(ThroughputPublisher Throughput_lib VortexDDS::vddsc)
target_link_libraries(ThroughputSubscriber Throughput_lib VortexDDS::vddsc)
