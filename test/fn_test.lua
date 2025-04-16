package.path = package.path  .. ";scene/lib//?.lua"
require 'scene/lib/common'

local registry = {
  updaters = {}
}
function registry:add_updater(fn)
  self.updaters[#self.updaters + 1] = fn
end

function registry:update(dt)
  for i, updater in ipairs(self.updaters) do
    updater(dt)
  end
end

-- function class_inherit(Base, Class)
--   setmetatable(Class, Base)
--   Class.__index = Class
--   return Class
-- end

function class_create(Actor, o)
  o = o or {}
  setmetatable(o, Actor)
  Actor.__index = Actor
  return o
end

function actor_create(Actor, o)
  o = o or {}
  setmetatable(o, Actor)
  Actor.__index = Actor
  if o.init then
    o:init()
  end
  return o
end

types = {}
classes = {}

local type_id = 1
types[type_id] = {}
classes[type_id] = {}

types[type_id]["fn_1_1"] = function(self)
  print(self)
  print("here-fn ")
  print(table_format(self))
end

table_print(types)

local function fn1(self)
  return self
end

print("here-2")

local f = fn1()
table_print(f)

print("here-3")
table_print(types[type_id])
print("here-3.1")

local n = actor_create(types[type_id])
print(n)
print("here-4.1")
table_print(n)
print("here-4.2")

print("here-5.1")
n:fn_1_1()
print("here-5.2")


local function class_1_2()
  local Actor = {}
  function Actor:init(state)
    print("init")
    self.cls_1 = 2
  end
  function Actor:start(state)
  end
  function Actor:update(dt, state)
    print("-OBJ--\n"..table_format(self))
    print("-STATE--\n"..table_format(state))
    printf("update: %f\n", dt)
  end
  return Actor
end

classes[type_id]["class_1_2"] = class_create(class_1_2())

n.obj_1_2 = actor_create(classes[type_id]["class_1_2"])

n.state_a = 1

print("here-6.1")
table_print(n)
print("here-6.2")

n.obj_1_2:init(1.2, n)
n.obj_1_2:start(1.2, n)

print("here-7.1")
n.obj_1_2:update(1.2, n)
print("here-7.2")


local function actor_1_2(state)
  local function update(dt)
    -- print("-OBJ--\n"..table_format(self))
    print("-STATE--\n"..table_format(state))
    printf("update: %f\n", dt)
  end

  registry:add_updater(update)
end

actor_1_2(n)
table_print(registry)

registry:update(1.22)
