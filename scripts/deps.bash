#!/usr/bin/env bash
# Builds a GCC cross-compiler
# See: https://wiki.osdev.org/GCC_Cross-Compiler
# TODO: eventually - https://wiki.osdev.org/OS_Specific_Toolchain

## shellcheck can't figure out that the return stmt won't actually make the code unreachable
# TODO: open github issue and/or fix

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

BINUTILS_VERSION='2.43'
GCC_VERSION='14.2.0'

GNU_FTP_URL='https://ftp.gnu.org'

TARGET='i686-elf'

# Build deps - gitignore this
COMPILER_DIR="$(pwd)/cross"
# Where we'll store the deps we need - primarily gcc and binutils
TOOLS_DIR="$COMPILER_DIR/src"
# Where we'll install the cross-compiler
BUILD_DIR="$COMPILER_DIR/opt/cross"

BINUTILS_BUILD_DIR="$TOOLS_DIR/build-binutils"
BINUTILS_OUT_DIR="binutils-$BINUTILS_VERSION"
BINUTILS_TAR="$BINUTILS_OUT_DIR.tar.xz"

GCC_BUILD_DIR="$TOOLS_DIR/build-gcc"
GCC_OUT_DIR="gcc-$GCC_VERSION"
GCC_TAR="$GCC_OUT_DIR.tar.xz"

DEPS=(
  # TODO: use att syntax
  'nasm'
  'qemu-system-x86'
  'qemu-ui-gtk'
  'base-devel'
  'gmp'
  'libmpc'
  'mpfr'
)

install_base_deps () {
  echo 'Installing base deps...'
  for dep in "${DEPS[@]}"; do
    # TODO: err if no yay
    # TODO: support other distros
    yay -S "$dep"
  done
}

setup_binutils () {
  echo 'Setting up binutils...'

  dir? "$BINUTILS_OUT_DIR" && {
    echo "$BINUTILS_OUT_DIR already setup; continuing to next step"
    return
  }

  mkdir -p "$BINUTILS_BUILD_DIR"

  cd /tmp || die 'Failed to cd to /tmp'
  fetch "$GNU_FTP_URL/gnu/binutils/$BINUTILS_TAR"
  tar -xJvf "$BINUTILS_TAR" -C "$TOOLS_DIR"

  cd "$BINUTILS_BUILD_DIR" || die "Failed to cd to $BINUTILS_BUILD_DIR"

  echo 'Configuring and installing binutils...'
  ../"$BINUTILS_OUT_DIR/configure" --target="$TARGET" --prefix="$BUILD_DIR" --with-sysroot --disable-nls --disable-werror
  make
  make install
}

setup_gcc () {
  echo 'Setting up gcc...'

  dir? "$GCC_OUT_DIR" && {
    echo "$GCC_OUT_DIR already setup; continuing to next step"
    return
  }

  mkdir -p "$GCC_BUILD_DIR"

  cd /tmp || die
  fetch "$GNU_FTP_URL/gnu/gcc/gcc-$GCC_VERSION/$GCC_TAR"
  tar -xJvf "$GCC_TAR" -C "$TOOLS_DIR"

  cd "$GCC_BUILD_DIR" || die "Failed to cd to $GCC_BUILD_DIR"

  # The $BUILD_DIR/bin dir *must* be in the PATH. We did that above.
  which -- "$TARGET-as" || echo "$TARGET-as is not in the PATH"

  echo 'Configuring and installing gcc...'
  ../"$GCC_OUT_DIR/configure" --target="$TARGET" --prefix="$BUILD_DIR" --disable-nls --enable-languages=c,c++ --without-headers
  make all-gcc
  make all-target-libgcc
  make install-gcc
  make install-target-libgcc
}

main () {
  reachable? "$GNU_FTP_URL" || {
    die "$GNU_FTP_URL not available; time to update this script (or your net is down)"
  }

  create_dir? "$TOOLS_DIR"
  create_dir? "$BUILD_DIR"

  export PATH="$BUILD_DIR/bin:$PATH"

  install_base_deps
  setup_binutils
  # WARNING: building this will make your laptop hot af
  # so don't fall asleep with it on your lap like I did
  setup_gcc
}

main

# ./cross/opt/cross/bin/i686-elf-gcc --version
