local Node = {}

function Node:new_class(o)
  o = o or {}
  setmetatable(o, self)
  self.__index = self
  return o
end

function Node:new(o)
  o = o or {}
  setmetatable(o, self)
  self.__index = self
  return o
end

function Node:destroy(o)
  printf("destroy node: %d\n", self.id)

  -- NOTE KI "local cmd" is backward compatibility trick in ScriptSystem
  states[self.id] = nil
  states[self.cmd] = nil
  states[self.node] = nil

  if self.updater_ids then
    for _, updater_id in ipairs(self.updater_ids) do
      Updater:remove_updater(updater_id)
    end
    self.updater_ids = nil
  end
  self.updaters = nil

  if self.listener_ids then
    for _, listener_id in ipairs(self.listener_ids) do
      events:unlisten(self.listener_id)
    end
    self.listener_ids = nil
  end
  self.listeners = nil
end

function Node:class()
  return getmetatable(self)
end

function Node:add_updater(fn)
  -- printf("NODE: %d: Add updater: %s\n", self.id, fn)
  self.updaters = self.updaters or {}
  self.updaters[#self.updaters + 1] = fn
end

-- function Node:ditto()
--   print("DITTO")
-- end

-- function Node:start()
-- end

-- function Node:update(dt)
-- end

function node_create(Actor, o)
  o = o or {}
  setmetatable(o, Actor)
  Actor.__index = Actor
  return o
end

return Node
