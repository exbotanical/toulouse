symbol-file kernel.sym
target remote localhost:1234

set pagination off
set logging file gdb.log
set logging enabled on

define hook-quit
  set logging off
  set confirm off
  detach
  # TODO: Find a better way to do this :(
  shell pkill -f qemu-system-x86_64
end
