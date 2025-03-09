print("LUA: MODULE common")

-- https://stackoverflow.com/questions/17877224/how-to-prevent-a-lua-script-from-failing-when-a-require-fails-to-find-the-scri
local function prequire(m)
  print("LUA::REQUIRE: " .. m)
  local ok, err = pcall(require, m)
  if not ok then
    print("LUA: MODULE_LOAD_ERROR: " .. string.format("%s", err))
    return nil, err
  end
  return err
end

prequire "util"
prequire "debug"
prequire "events"
prequire "module_1"

_G.math3d = prequire "math3d"
print(math3d)
-- print(format_table(math3d))
local vec = math3d.vector(1,1,1)
printf("math3d.length=%f\n", math3d.length(vec))
print("math3d OK")

local function test()
  print("LUA: testing...")
  local delay = 1.0
  local id = 10
  printf("LUA: TEST: delay=%f, id=%i\n", delay, id)
  print("LUA: tested!!!")
end

test()

print("LUA: COMMON: ready")
