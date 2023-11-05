#!/usr/bin/env bash

deps=(
  'nasm'
  'qemu-system-x86'
  'qemu-ui-gtk'
)

main () {
  for dep in "${deps[@]}"; do
    yay -S $dep
  done
}

main
