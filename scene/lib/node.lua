local Node = {}

function Node:new(o)
  o = o or {}
  setmetatable(o, self)
  self.__index = self
  return o
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
