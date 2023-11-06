#!/usr/bin/env bash
TOOLS_DIR="$HOME/src"

BINUTILS_BUILD_DIR="$TOOLS_DIR/build-binutils"
BINUTILS_OUT_DIR='binutils-2.41'
BINUTILS_TAR="$BINUTILS_OUT_DIR.tar.xz"

GCC_BUILD_DIR="$TOOLS_DIR/build-gcc"
GCC_OUT_DIR='gcc-12.2.0'
GCC_TAR="$GCC_OUT_DIR.tar.xz"

deps=(
  'nasm'
  'qemu-system-x86'
  'qemu-ui-gtk'
  'base-devel'
  'gmp'
  'libmpc'
  'mpfr'
)

install_base_deps () {
  for dep in "${deps[@]}"; do
    yay -S $dep
  done
}

setup_binutils () {
  if [[ -d $BINUTILS_BUILD_DIR ]]; then
    return
  fi

  mkdir -p $BINUTILS_BUILD_DIR

  cd /tmp || exit 1
  curl -O https://ftp.gnu.org/gnu/binutils/$BINUTILS_TAR

  tar -xJvf $BINUTILS_TAR -C $HOME/src

  cd $BINUTILS_BUILD_DIR || exit 1

  ../$BINUTILS_OUT_DIR/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
  make
  make install
}

setup_gcc () {
  if [[ -d $GCC_BUILD_DIR ]]; then
    return
  fi

  mkdir -p $GCC_BUILD_DIR

  cd /tmp || exit 1
  curl -O https://mirrors.concertpass.com/gcc/releases/gcc-12.2.0/$GCC_TAR

  tar -xJvf $GCC_TAR -C $HOME/src

  cd $GCC_BUILD_DIR || exit 1

  # The $PREFIX/bin dir _must_ be in the PATH. We did that above.
  which -- $TARGET-as || echo $TARGET-as is not in the PATH

  cd $GCC_BUILD_DIR || exit 1

  ../$GCC_OUT_DIR/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
  make all-gcc
  make all-target-libgcc
  make install-gcc
  make install-target-libgcc
}

# https://wiki.osdev.org/GCC_Cross-Compiler
main () {
  export PREFIX="$HOME/opt/cross"
  export TARGET=i686-elf
  export PATH="$PREFIX/bin:$PATH"

  # install_base_deps
  # setup_binutils
  # setup_gcc
}

main
