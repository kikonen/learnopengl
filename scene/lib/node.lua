local Node = {}

function Node:new(o)
  o = o or {}
  setmetatable(o, self)
  self.__index = self
  return o
end

function Node:destroy(o)
  printf("destroy node: %d\n", self.id)

  states[self.id] = nil

  -- TODO KI how to find updater?!?
  Updater:remove_updater(self)

  -- TODO KI how to find listener_id?!?
  events:unlisten(self.listener_id)
end

function Node:class()
  return getmetatable(self)
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
