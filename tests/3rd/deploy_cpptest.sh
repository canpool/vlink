#!/bin/sh
CUR_DIR="$(dirname "$(readlink -f "$0")")"
cd "$CUR_DIR"

# params
ARCH_TYPE="32" # 32/64
INSTALL_DIR=cpptest

Usage() {
	echo "Usage:"
	echo "	${SCRIPT_NAME} [-h] [-m 32|64] args ..."
	echo ""
	echo "OPTIONS"
	echo "	-m <32|64>       Generate code for a 32-bit or 64-bit environment"
	echo "	-d <dir>         Specify installation directory"
	echo "	-h               Show help message"
	echo ""
} 

# getopts
while getopts "m:d:h" opt; do
	case $opt in
		m)
			ARCH_TYPE=$OPTARG
			echo "ARCH_TYPE: $ARCH_TYPE"
			;;
		d)
			INSTALL_DIR=$OPTARG
			echo "INSTALL_DIR: $INSTALL_DIR"
			;;
		h)
			Usage
			;;
		\?)
			Usage
			exit -1
			;;
	esac
done

# check
if [ -d "${INSTALL_DIR}" ]; then
	echo "cpptest is deployed in ${INSTALL_DIR}"
	exit
fi

# cpptest
CPPTEST_BASE_NAME=cpptest-1.1.2
CPPTEST_FILE=${CPPTEST_BASE_NAME}.tgz
if [ ! -d ${CPPTEST_BASE_NAME} ]; then
	if [ ! -e ${CPPTEST_FILE} ];then
		#wget -c https://github.com/cpptest/cpptest/archive/1.1.2.zip -O ${CPPTEST_FILE}
		wget -c https://github.com/canpool/tools/raw/master/OpenSources/cpptest-1.1.2.tgz -O ${CPPTEST_FILE}
		if [ $? != 0 ]; then
			echo "Err: download cpptest failed!"
			exit -2
		fi
	fi
	tar -xzf ${CPPTEST_FILE}
fi

# flags
if [ "X$ARCH_TYPE" = "X32" ];then
ARCH_FLAGS="CCFLAGS=-m32 CPPFLAGS=-m32 CXXFLAGS=-m32 CFLAGS=-m32 LDFLAGS=-m32"
fi

# dir
SRC_DIR=$CUR_DIR/$CPPTEST_BASE_NAME
INSTALL_DIR=$CUR_DIR/$INSTALL_DIR

# clean
rm -rf $INSTALL_DIR

# build
cd "$SRC_DIR"
if [ -e Makefile ]; then
	make clean
fi
./configure --prefix=$INSTALL_DIR $ARCH_FLAGS
make
make install

# return
cd "$CUR_DIR"
