#!/usr/bin/env bash
# shellcheck disable=SC2317

# shellcheck disable=2128
ROOT_DIR="$(dirname "$(readlink -f $BASH_SOURCE)")"
source "$ROOT_DIR/utils.bash"

# stop here if being sourced
return 2>/dev/null

# stop on errors and unset variable refs
set -o errexit
set -o errtrace
set -o nounset
set -o pipefail

BINUTILS_VERSION='2.41'
GCC_VERSION='12.2.0'

# Where we'll store the deps we need - primarily gcc and binutils
TOOLS_DIR='./deps/src'
# Where we'll install the cross-compiler
BUILD_DIR='./deps/opt/cross'

BINUTILS_BUILD_DIR="$TOOLS_DIR/build-binutils"
BINUTILS_OUT_DIR="binutils-$BINUTILS_VERSION"
BINUTILS_TAR="$BINUTILS_OUT_DIR.tar.xz"

GCC_BUILD_DIR="$TOOLS_DIR/build-gcc"
GCC_OUT_DIR="gcc-$GCC_VERSION"
GCC_TAR="$GCC_OUT_DIR.tar.xz"

DEPS=(
  'nasm'
  'qemu-system-x86'
  'qemu-ui-gtk'
  'base-devel'
  'gmp'
  'libmpc'
  'mpfr'
)

install_base_deps () {
  for dep in "${DEPS[@]}"; do
    yay -S "$dep"
  done
}

setup_binutils () {
  if [[ -d "$BINUTILS_BUILD_DIR" ]]; then
    return
  fi

  mkdir -p "$BINUTILS_BUILD_DIR"

  cd /tmp || die
  curl -O "https://ftp.gnu.org/gnu/binutils/$BINUTILS_TAR"

  tar -xJvf "$BINUTILS_TAR" -C "$HOME/src"

  cd "$BINUTILS_BUILD_DIR" || die

  ../"$BINUTILS_OUT_DIR/configure" --target="$TARGET" --prefix="$BUILD_DIR" --with-sysroot --disable-nls --disable-werror
  make
  make install
}

setup_gcc () {
  if [[ -d "$GCC_BUILD_DIR" ]]; then
    return
  fi

  mkdir -p "$GCC_BUILD_DIR"

  cd /tmp || die
  curl -O "https://mirrors.concertpass.com/gcc/releases/$GCC_OUT_DIR/$GCC_TAR"

  tar -xJvf "$GCC_TAR" -C "$HOME/src"

  cd "$GCC_BUILD_DIR" || die

  # The $BUILD_DIR/bin dir _must_ be in the PATH. We did that above.
  which -- "$TARGET-as" || echo "$TARGET-as" is not in the PATH

  cd "$GCC_BUILD_DIR" || die

  ../"$GCC_OUT_DIR/configure" --target="$TARGET" --prefix="$BUILD_DIR" --disable-nls --enable-languages=c,c++ --without-headers
  make all-gcc
  make all-target-libgcc
  make install-gcc
  make install-target-libgcc
}

# https://wiki.osdev.org/GCC_Cross-Compiler
main () {
  create_dir? "$TOOLS_DIR"
  create_dir? "$BUILD_DIR"

  export TARGET='i686-elf'
  export PATH="$BUILD_DIR/bin:$PATH"

  install_base_deps
  setup_binutils
  setup_gcc
}

main
