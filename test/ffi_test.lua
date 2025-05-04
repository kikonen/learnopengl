local profile = require("jit.profile")

local function cb(thread, samples, vmstate)
  print(thread, samples, vmstate)
end

profile.start("l", cb)

local ffi = require("ffi")
ffi.cdef[[
int MessageBoxA(void *w, const char *txt, const char *cap, int type);
]]
ffi.C.MessageBoxA(nil, "Hello world!", "Test", 0)

print(jit.status())

profile.stop()

-- dump = profile.dumpstack("thread", "f\n", 100)

-- print(dump)

print(_G)
