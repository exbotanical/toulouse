
# create_dir? creates the given directory if it doesn't exist
create_dir? () {
  local dir="$1"
  if [[ -d "$dir" ]]; then
    echo "Missing $dir; creating..."
    mkdir -p "$dir"
  fi
}

# die prints a message to stderr and exits with either
# the given return code or 1 if none was provided
die() {
  local msg="$1"
  local rc="${2:-1}"
  >&2 echo "$msg"
  exit "$rc"
}
