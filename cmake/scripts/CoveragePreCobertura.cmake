#
# This script assumes that it is called before all tests are run and gcov results are available.
# It can be used to setup the environment needed to get proper Cobertura coverage results.
#
# Example usage:
# $ cmake -DSOURCE_DIR=<cham src> -DTEST_DIR=<cham bld> -DOUTPUT_DIR=<output dir> -P <cham src>/cmake/scripts/CoveragePreCobertura.cmake
# $ ctest -T test
# $ ctest -T coverage
# $ ctest -DSOURCE_DIR=<cham src> -DTEST_DIR=<cham bld> -DOUTPUT_DIR=<output dir> -P <cham src>/cmake/scripts/CoveragePostCobertura.cmake

#
# Nothing to do really.
# This is just added to provide consistency between Coverage scripts.
#

