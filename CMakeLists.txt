CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7)
INCLUDE(ExternalProject)
PROJECT(BEBOT_POWERBOARD_BSP)

#
# Options
#
SET(BINUTILS_VERSION "2.18" CACHE STRING "Enter the Binutils version to build")
SET(GMP_VERSION "4.3.2" CACHE STRING "Enter the GMP version for GCC")
SET(MPFR_VERSION "2.4.2" CACHE STRING "Enter the MPFR version for GCC")
SET(GCC_VERSION "4.3.2" CACHE STRING "Enter the GCC version to build")

#
# Build the toolchain
#
ADD_SUBDIRECTORY(toolchain)

