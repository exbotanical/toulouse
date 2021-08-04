#!/usr/bin/env bash
#desc           :Build the toulouse C cross-compiler
#author         :Matthew Zito (goldmund)
#created        :8/2021
#version        :1.0.0
#usage          :./build_cc.bash
#environment    :5.0.17(1)-release
#===============================================================================

setup () {
  export PREFIX="$HOME/opt/cross"
  export TARGET=i686-elf
  export PATH="$PREFIX/bin:$PATH"

  mkdir -p $HOME/src/build-binutils $HOME/src/build-gcc

  curl -L $BINUTILS_URL > $DOWNLOADS_DIR/binutils-$BINUTILS_VERSION.tar.xz || panic $E_COMMAND "failed to download binutils"
  chk_file $DOWNLOADS_DIR/binutils-$BINUTILS_VERSION.tar.xz 

  curl -L $GCC_URL > $DOWNLOADS_DIR/gcc-$GCC_VERSION.tar.gz || panic $E_COMMAND "failed to download gcc"
  chk_file $DOWNLOADS_DIR/gcc-$GCC_VERSION.tar.gz

  tar -xf $DOWNLOADS_DIR/binutils-$BINUTILS_VERSION.tar.xz -C $HOME/src || panic $E_COMMAND "failed to unzip binutils"
  tar -xf $DOWNLOADS_DIR/gcc-$GCC_VERSION.tar.gz -C $HOME/src || panic $E_COMMAND "failed to unzip gcc"
}

main () {
  setup

  chdir_and_chk $HOME/src

  chdir_and_chk build-binutils

  ../binutils-$BINUTILS_VERSION/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
  make
  make install

  chdir_and_chk $HOME/src
  
  # The $PREFIX/bin dir _must_ be in the PATH. We did that above.
  which -- $TARGET-as || echo $TARGET-as is not in the PATH

  chdir_and_chk build-gcc

  ../gcc-$GCC_VERSION/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
  make all-gcc
  make all-target-libgcc
  make install-gcc
  make install-target-libgcc
}

##############################
###         Sources        ###
##############################

source "$(dirname $0)/util.bash"

##############################
###        Constants       ###
##############################

BINUTILS_VERSION='2.35'
GCC_VERSION='10.2.0'

DOWNLOADS_DIR="$HOME/Downloads"

BINUTILS_URL="https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.xz"
GCC_URL="https://mirrorservice.org/sites/sourceware.org/pub/gcc/releases/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.gz"

##############################
###      *Begin Exec*      ###
##############################

# stop here if being sourced
return 2>/dev/null

# stop on errors
set -o errexit
# stop on unset variable refs
set -o nounset

main
