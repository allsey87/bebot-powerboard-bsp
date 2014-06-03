#!/bin/bash

#
# Toolchain component version numbers
#

# binutils 2.24 was released on 02/12/2013
# gcc 4.8.2 was released on 16/10/2013
BINUTILS_VERSION="2.24"
GCC_VERSION="4.8.2"
GMP_VERSION="6.0.0a"
MPFR_VERSION="3.1.2"
MPC_VERSION="1.0.2"
AVR_LIBC_VERSION="1.8.0"

#
# Directories
#

SOURCE_DIR=${PWD}/toolchain/source
DOWNLOAD_DIR=${PWD}/toolchain/download
BUILD_DIR=${PWD}/toolchain/build
INSTALL_DIR=${PWD}/toolchain/install

#
# Download sources
#
echo "== Downloading archives to ${DOWNLOAD_DIR} =="

wget -N ftp://gcc.gnu.org/pub/gcc/releases/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.bz2 -P ${DOWNLOAD_DIR}
wget -N http://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.bz2 -P ${DOWNLOAD_DIR}
wget -N https://gmplib.org/download/gmp/gmp-${GMP_VERSION}.tar.bz2 -P ${DOWNLOAD_DIR}
wget -N http://www.mpfr.org/mpfr-${MPFR_VERSION}/mpfr-${MPFR_VERSION}.tar.bz2 -P ${DOWNLOAD_DIR}
wget -N ftp://ftp.gnu.org/gnu/mpc/mpc-${MPC_VERSION}.tar.gz -P ${DOWNLOAD_DIR}
wget -N http://download.savannah.gnu.org/releases/avr-libc/avr-libc-${AVR_LIBC_VERSION}.tar.bz2 -P ${DOWNLOAD_DIR}

#
# Extract sources for in tree build
#
echo "== Extracting sources to ${SOURCE_DIR} =="

mkdir -pv ${SOURCE_DIR}/gcc
mkdir -pv ${SOURCE_DIR}/gcc/gmp
mkdir -pv ${SOURCE_DIR}/gcc/mpfr
mkdir -pv ${SOURCE_DIR}/gcc/mpc
mkdir -pv ${SOURCE_DIR}/avrlibc

tar jxf ${DOWNLOAD_DIR}/gcc-${GCC_VERSION}.tar.bz2 -C ${SOURCE_DIR}/gcc --strip-components=1
tar kjxf ${DOWNLOAD_DIR}/binutils-${BINUTILS_VERSION}.tar.bz2 -C ${SOURCE_DIR}/gcc --strip-components=1
tar jxf ${DOWNLOAD_DIR}/gmp-${GMP_VERSION}.tar.bz2 -C ${SOURCE_DIR}/gcc/gmp --strip-components=1
tar jxf ${DOWNLOAD_DIR}/mpfr-${MPFR_VERSION}.tar.bz2 -C ${SOURCE_DIR}/gcc/mpfr --strip-components=1
tar zxf ${DOWNLOAD_DIR}/mpc-${MPC_VERSION}.tar.gz -C ${SOURCE_DIR}/gcc/mpc --strip-components=1
tar jxf ${DOWNLOAD_DIR}/avr-libc-${AVR_LIBC_VERSION}.tar.bz2 -C ${SOURCE_DIR}/avrlibc --strip-components=1

#
# Build GCC and Binutils
#
echo "== Building GCC and Binutils =="

mkdir -pv ${BUILD_DIR}/gcc
mkdir -pv ${INSTALL_DIR}/gcc
cd ${BUILD_DIR}/gcc
${SOURCE_DIR}/gcc/configure --prefix=${INSTALL_DIR}/gcc --target=avr --enable-languages=c,c++ --disable-nls --with-dwarf2
make -j4
make install

#
# Build AVR LIBC
#
echo "== Building AVR LIBC =="

PATH=${INSTALL_DIR}/gcc/bin:${PATH}
mkdir -pv ${BUILD_DIR}/avrlibc
mkdir -pv ${INSTALL_DIR}/avrlibc
cd ${BUILD_DIR}/avrlibc
${SOURCE_DIR}/avrlibc/configure --prefix=${INSTALL_DIR}/avrlibc --build=`${SOURCE_DIR}/avrlibc/config.guess` --host=avr
pause
make -j4
make install




