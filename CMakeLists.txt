CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7)
INCLUDE(ExternalProject)
PROJECT(BEBOT_POWERBOARD_BSP)

#
# Options
#
SET(BINUTILS_VERSION "2.18" CACHE STRING "Enter the GNU Binutils version to build")
SET(GCC_VERSION "4.3.2" CACHE STRING "Enter the GCC version to build")

#
# Build the toolchain
#
ADD_SUBDIRECTORY(toolchain)

