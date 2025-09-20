local State = {}

function State:new_class(o)
  o = o or {}
  setmetatable(o, self)
  self.__index = self
  return o
end

function State:new(o)
  o = o or {}
  setmetatable(o, self)
  self.__index = self
  return o
end

function State:_init(o)
  if not self._initialized and self.initialize then
    self:initialize()
    self._initialized = true
  end
end

function State:destroy(o)
  debug("destroy state: %s\n", self.handle)

  states[self.handle] = nil
  states[self.state] = nil

  self.updaters = nil

  if self.listener_ids then
    for _, listener_id in ipairs(self.listener_ids) do
      events:unlisten(listener_id)
    end
    self.listener_ids = nil
  end
end

function State:class()
  return getmetatable(self)
end

function State:add_updater(fn)
  -- debug("STATE: %s: Add updater: %s\n", self.handle, fn)
  self.updaters = self.updaters or {}
  self.updaters[#self.updaters + 1] = fn
end

function State:listen(fn, types)
  listener_id = events:listen(fn, types)

  debug("STATE: %s: listen: listener=%d, fn=%s, types={%s}\n", self.handle, listener_id, fn, table_format(types))

  self.listener_ids = self.listener_ids or {}
  self.listener_ids[#self.listener_ids + 1] = listener_id

  return listener_id
end

-- function State:ditto()
--   print("DITTO")
-- end

-- function State:start()
-- end

-- function State:update(dt)
-- end

function state_create(Actor, o)
  o = o or {}
  setmetatable(o, Actor)
  Actor.__index = Actor
  return o
end

return State
