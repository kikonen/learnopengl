local BaseState = {}

-- New node type specific state class
-- @param o { type_id: }
function BaseState:new_class(o)
  o = o or {}
  setmetatable(o, self)
  self.__index = self
  return o
end

-- new state instance
function BaseState:new(o)
  o = o or {}
  setmetatable(o, self)
  self.__index = self
  return o
end

-- one time init for State instance
function BaseState:_init()
  if not self._initialized and self.initialize then
    self:initialize()
    self._initialized = true
  end
end

function BaseState:destroy()
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

-- get class for state
function BaseState:class()
  return getmetatable(self)
end

function BaseState:add_updater(fn)
  -- debug("STATE: %s: Add updater: %s\n", self.handle, fn)
  self.updaters = self.updaters or {}
  self.updaters[#self.updaters + 1] = fn
end

function BaseState:listen(fn, types)
  listener_id = events:listen(fn, types)

  debug("STATE: %s: listen: listener=%d, fn=%s, types={%s}\n", self.handle, listener_id, fn, table_format(types))

  self.listener_ids = self.listener_ids or {}
  self.listener_ids[#self.listener_ids + 1] = listener_id

  return listener_id
end

local function _create_state(id, typeId)
  states[id] = classes[typeId]:new()
end

local function _destroy_state(id)
  print("-------")
  print(states)
  print("-------")
  if states[id] then
    states[id]:destroy()
    states[id] = nil
  end
end

_G._create_state = _create_state
_G._destroy_state = _destroy_state

return BaseState
