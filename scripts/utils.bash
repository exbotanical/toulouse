# dir? says whether a given directory path exists
dir? () {
  [[ -d "$1" ]]
}

# create_dir? creates the given directory if it doesn't exist
create_dir? () {
  local dir="$1"
  if ! dir? "$dir"; then
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

# reachable? makes a HEAD request to check if the given URL is available
reachable? () {
  local url="$1"
  curl -s -o /dev/null -I -w "%{http_code}" "$url" | grep -q '200'
}

# fetch curls a url path into the cwd
fetch () {
  local url="$1"
  echo "Curling $url..."
  curl -O "$url"
}
