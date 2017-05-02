#
# This script assumes that it is called before all tests are run and gcov results are available.
# It can be used to setup the environment needed to get proper Cobertura coverage results.
#
# Example usage:
# $ cmake -DCOVERAGE_CONFIG=<cham bld>/CoverageConfig.cmake -P <cham src>/cmake/scripts/CoveragePreCobertura.cmake
# $ ctest -T test
# $ ctest -T coverage
# $ ctest -DCOVERAGE_CONFIG=<cham bld>/CoverageConfig.cmake -P <cham src>/cmake/scripts/CoveragePostCobertura.cmake
# If you start the scripts while in <cham bld> then you don't have to provide the COVERAGE_CONFIG file.
#
cmake_minimum_required(VERSION 3.5)

#
# Nothing to do really.
# This is just added to provide consistency between Coverage scripts.
#

