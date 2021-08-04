#!/usr/bin/env bash
#desc           :Bash utilities
#author         :Matthew Zito (goldmund)
#created        :8/2021
#version        :1.0.0
#usage          :source util.bash
#environment    :5.0.17(1)-release
#===============================================================================

OLD_IFS=$IFS
IFS=$'\n'

# exit codes
E_XCD=86
E_CANTCREATE=76
E_CANTCP=77
E_USAGE=64
E_NOTROOT=87
E_COMMAND=1

# constants
ROOT_UID=0

now () {
  printf '%(%I:%M %p)T\n'
}

panic () {
  local exit_status=$1

  shift # pop exit status; we don't want to print it

  echo "[-] ERROR ($(now)): $*" >&2
  exit $exit_status
}

chk_dir () {
  local target_dir=$1

  if [[ $(pwd) != $target_dir ]]; then
    panic $E_XCD "Unable to 'cd' into $target_dir"
  fi
}

chdir_and_chk () {
  local target_dir=$1

  cd $target_dir 
  chk_dir $target_dir
}

chk_root () {
  if [[ ! $UID -eq $ROOT_UID ]]; then
    panic $E_NOTROOT "Must execute as root"
  fi
}

chk_file () {
  local target_file=$1

  if [[ ! -e $target_file ]];then
    panic $E_FILENOTFOUND "Hosts file not found"
  fi
}

chk_ret () {
  local callback=$1

  shift

  local args=$@

  (( !$? )) && $callback $args
}

IFS=$OLD_IFS
