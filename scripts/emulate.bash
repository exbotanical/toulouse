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
}

main () {
  setup
  make emu
}

##############################
###         Sources        ###
##############################

source "$(dirname $0)/util.bash"

##############################
###        Constants       ###
##############################


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
