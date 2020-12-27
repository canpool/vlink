#!/bin/sh
# Copyright (c) [2020] vlink Team. All rights reserved.
# SPDX-License-Identifier: MulanPSL-2.0

CUR_DIR="$(dirname "$(readlink -f "$0")")"

# base info
NAME=googletest
VERSION=1.10.0
URL=https://github.com/google/googletest
SOURCE0=${URL}/archive/release-${VERSION}/${NAME}-release-${VERSION}.tar.gz
SRC_NAME=${NAME}-release-${VERSION}
TAR_BALL=${SRC_NAME}.tar.gz
# dir
INSTALL_DIR=$CUR_DIR/$NAME
SRC_DIR=$CUR_DIR/$SRC_NAME
BUILD_DIR=$SRC_DIR/build
LIB32_DIR=$INSTALL_DIR/lib32
LIB64_DIR=$INSTALL_DIR/lib64

prepare() {
    # check install dir
    if [ -d "${INSTALL_DIR}" ]; then
        echo "googletest is deployed in ${INSTALL_DIR}"
        echo "You can remove and redeploy it!"
        exit
    fi
    # remove src dir
    if [ -d "${SRC_DIR}" ]; then
        rm -rf ${SRC_DIR}
    fi

    if [ ! -f "${TAR_BALL}" ]; then
        wget "${SOURCE0}"
        if [ $? != 0 ]; then
            echo "error: download googletest failed!"
            exit
        fi
    fi
    tar -zxf ${TAR_BALL}
}

# build(arch_bits)
build() {
    # mk
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    # flags
    if [ "X$1" = "X32" ]; then
        ARCH_FLAGS="-DCMAKE_CXX_FLAGS=-m32"
        LIB_DIR=$LIB32_DIR
    else
        ARCH_FLAGS="-DCMAKE_CXX_FLAGS=-m64"
        LIB_DIR=$LIB64_DIR
    fi

    cmake ${ARCH_FLAGS} ../
    if [ $? != 0 ]; then
        echo "error: cmake failed!"
        exit
    fi
    make clean
    make

    # copy
    A_FILES="$(find ./ -name *.a)"
    mkdir -p "$LIB_DIR"
    cp -rf ${A_FILES} "$LIB_DIR"

    cd -
    # rm
    rm -rf "$BUILD_DIR"
}

install() {
    cp -rf ${SRC_DIR}/googlemock/include "$INSTALL_DIR"
    cp -rf ${SRC_DIR}/googletest/include "$INSTALL_DIR"
}

main() {
    cd "$CUR_DIR"

    # pre
    prepare

    # build
    build 32
    build 64

    # install
    install
}

main "$@"
