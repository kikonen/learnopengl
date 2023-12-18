require "module_1"

print("COMMON: init")

function printf(...)
  io.write(string.format(...))
end

print("testing...")
local delay = 1.0
local id = 10
printf("TEST: delay=%f, id=%i\n", delay, id)
print("tested!!!")

print("COMMON: ready")
