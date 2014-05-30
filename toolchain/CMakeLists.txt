#
# Build toolchain for AVR
#
EXTERNALPROJECT_ADD(gcc
	PREFIX gcc
	URL ftp://gcc.gnu.org/pub/gcc/releases/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.bz2
	CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> --target=avr --disable-nls --disable-werror
	BUILD_COMMAND make MAKEINFO=true
	INSTALL_COMMAND make install MAKEINFO=true
)
ExternalProject_Get_Property(gcc DOWNLOAD_DIR SOURCE_DIR)

#
# Fetch and merge Binutils sources
#
ExternalProject_Add_Step(gcc binutils-fetch
	COMMAND wget -N http://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.bz2
	WORKING_DIRECTORY ${DOWNLOAD_DIR}
   DEPENDEES download
)

ExternalProject_Add_Step(gcc binutils-merge
   COMMAND tar kjxf ${DOWNLOAD_DIR}/binutils-${BINUTILS_VERSION}.tar.bz2 --strip-components=1	
	WORKING_DIRECTORY ${SOURCE_DIR}
   DEPENDEES binutils-fetch
   DEPENDERS configure
)

#
# Fetch and merge GMP sources
#
ExternalProject_Add_Step(gcc gmp-fetch
	COMMAND wget -N https://gmplib.org/download/gmp/gmp-${GMP_VERSION}.tar.bz2
	WORKING_DIRECTORY ${DOWNLOAD_DIR}
   DEPENDEES download
)

ExternalProject_Add_Step(gcc gmp-merge
   COMMAND mkdir gmp && tar kjxf ${DOWNLOAD_DIR}/gmp-${GMP_VERSION}.tar.bz2 -C ./gmp --strip-components=1
	WORKING_DIRECTORY ${SOURCE_DIR}
   DEPENDEES gmp-fetch
   DEPENDERS configure
)

#
# Fetch and merge MPFR sources
#
ExternalProject_Add_Step(gcc mpfr-fetch
	COMMAND wget -N http://www.mpfr.org/mpfr-${MPFR_VERSION}/mpfr-${MPFR_VERSION}.tar.bz2
	WORKING_DIRECTORY ${DOWNLOAD_DIR}
   DEPENDEES download
)

ExternalProject_Add_Step(gcc mpfr-merge
   COMMAND mkdir mpfr && tar kjxf ${DOWNLOAD_DIR}/mpfr-${MPFR_VERSION}.tar.bz2 -C ./mpfr --strip-components=1
	WORKING_DIRECTORY ${SOURCE_DIR}
   DEPENDEES mpfr-fetch
   DEPENDERS configure
)

