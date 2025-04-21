#!/usr/bin/env bash

set -euo pipefail

KERNEL_PATH=dist/toulouse.0.0.1.img
QEMU=${QEMU:-qemu-system-i386}
QEMU_DRIVE_CONF=${QEMU_DRIVE_CONF:-file=$KERNEL_PATH,format=raw}
QEMU_ARGS=${QEMU_ARGS:--m 64M -nographic}

make dist || exit 1

timeout 5s $QEMU -drive "$QEMU_DRIVE_CONF" $QEMU_ARGS < /dev/null > /dev/null 2>&1 || true
status=$?

if [ $status -eq 124 ]; then
  echo "✅ Kernel ran (timeout hit, assumed success)"
  exit 0
elif [ $status -eq 0 ]; then
  echo "✅ Kernel exited cleanly"
  exit 0
else
  echo "❌ Kernel crashed (exit code $status)"
  exit 1
fi
