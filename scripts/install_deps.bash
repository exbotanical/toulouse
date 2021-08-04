#!/usr/bin/env bash
#desc           :Install necessary packages (debian)
#author         :Matthew Zito (goldmund)
#created        :8/2021
#version        :1.0.0
#usage          :./install_deps.bash
#environment    :5.0.17(1)-release
#===============================================================================

main () {
  chk_root

  # debian
  for dep in ${DEPENDENCIES[@]}; do
    apt install "$dep"
  done
}

##############################
###         Sources        ###
##############################

source "$(dirname $0)/util.bash"

##############################
###        Constants       ###
##############################
DEPENDENCIES=(
  build-essential	
  bison 
  flex 
  libgmp3 
  libmpc-dev	
  libmpfr-dev	
  texinfo 
  libisl-dev	 
)

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
