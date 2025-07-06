print("LUA: MODULE events")

local id_base = 0

local EventQueue = {
  listeners = {},
  by_type = {}
}

local Event = {
  SCRIPT_RESUME = 10
}

local function next_id()
  id_base = id_base + 1
  return id_base
end

function EventQueue:clear()
  self.listeners = {}
  self.by_type = {}
end

function EventQueue:listen(listener, event_types)
  local listener_id = next_id()
  -- debug("EVENT::LISTEN: type=%s, listener_id=%d, listener=%s\n", ev_type, listener_id, listener)

  self.listeners[listener_id] = listener

  local a = self.by_type

  for i, ev in ipairs(event_types) do
    a[ev] = a[ev] or {}
    a[ev][listener_id] = true
  end

  return listener_id
end

function EventQueue:unlisten(listener_id)
  if not listener_id then return end

  debug("EVENT::UNLISTEN: listener=%d\n", listener_id)

  table.remove(self.listeners, listener_id)

  for ev_type, listener_ids in pairs(self.by_type) do
    if listener_ids[listener_id] then
      -- debug("EVENT::UNLISTEN_TYPE: type=%s, listener=%d\n", ev_type, listener_id)
      table.remove(listener_ids, listener_id)
    end
  end
end

function EventQueue:emit(event)
  self:emit_raw(event.type, event.data, event.listener_id)
end

function EventQueue:emit_raw(ev_type, data, listener_id)
  -- debug("EVENT::EMIT: listener=%s, event=%s, data=%s\n", listener_id, ev_type, data)
  local found = false

  local event = {
    type = ev_type,
    listener = listener_id,
    data = data
  }

  queue = self.by_type[ev_type]
  if queue then
    if listener_id then
      if queue[listener_id] then
        -- debug("EVENT::NOTIFY_LISTENER: type=%s, listener=%d\n", ev_type, listener_id)
        self.listeners[listener_id](event)
        found = true
      end
    else
      for lid, _ in pairs(queue) do
        -- debug("EVENT::NOTIFY_LISTENER: type=%s, listener=%d\n", ev_type, lid)
        self.listeners[lid](event)
        found = true
      end
    end
  end

  if not found then
    debug("EVENT::NOTIFY_NOT_FOUND: type=%s, listener=%s\n", ev_type, listener_id)
  end
end

_G["events"] = EventQueue
_G["Event"] = Event
