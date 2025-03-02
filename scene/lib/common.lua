print("LUA: MODULE common")

print("LUA: COMMON: init")

function printf(...)
  io.write(string.format(...))
end

print("LUA: testing...")
local delay = 1.0
local id = 10
printf("LUA: TEST: delay=%f, id=%i\n", delay, id)
print("LUA: tested!!!")

print("LUA: COMMON: ready")
