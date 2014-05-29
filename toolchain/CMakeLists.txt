#
# Build Binutils for AVR
#
EXTERNALPROJECT_ADD(binutils
	PREFIX binutils
	URL http://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.bz2
	CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> --target=avr --disable-nls --disable-werror
	BUILD_COMMAND make MAKEINFO=true
	INSTALL_COMMAND make install MAKEINFO=true
)
ExternalProject_Get_Property(binutils SOURCE_DIR INSTALL_DIR)
SET(BINUTILS_SOURCE_DIR ${SOURCE_DIR})
SET(BINUTILS_INSTALL_DIR ${INSTALL_DIR})

#
# Build GCC for AVR
#


EXTERNALPROJECT_ADD(gcc
	DEPENDS binutils
	PREFIX gcc
	URL ftp://gcc.gnu.org/pub/gcc/releases/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.bz2
	#CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> --target=avr
	#BUILD_COMMAND make
	#INSTALL_COMMAND make install
)
ExternalProject_Get_Property(gcc SOURCE_DIR INSTALL_DIR)
SET(GCC_SOURCE_DIR ${SOURCE_DIR})
SET(GCC_INSTALL_DIR ${INSTALL_DIR})

ExternalProject_Add_Step(gcc link
    COMMAND for file in ${BINUTILS_SOURCE_DIR}/* ; do ln -s "${file}" ; done
    DEPENDERS configure
)
