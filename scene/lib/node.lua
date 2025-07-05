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

  states[self.id] = nil
  states[self.cmd] = nil
  states[self.node] = nil

  self.updaters = nil

  if self.listener_ids then
    for _, listener_id in ipairs(self.listener_ids) do
      events:unlisten(self.listener_id)
    end
    self.listener_ids = nil
  end
end

function Node:class()
  return getmetatable(self)
end

function Node:add_updater(fn)
  -- debug("NODE: %d: Add updater: %s\n", self.id, fn)
  self.updaters = self.updaters or {}
  self.updaters[#self.updaters + 1] = fn
end

function Node:listen(fn, types)
  listener_id = events:listen(fn, types)

  debug("NODE: %d: listen: listener=%d, fn=%s, types={%s}\n", self.id, listener_id, fn, table_format(types))

  self.listener_ids = self.listener_ids or {}
  self.listener_ids[#self.listener_ids + 1] = listener_id

  return listener_id
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
