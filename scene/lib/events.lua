print("LUA: MODULE events")

local id_base = 0

local EventQueue = {
  listeners = {},
  by_type = {}
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
  printf("EVENT::LISTEN: type=%s, listener_id=%d, listener=%s\n", ev_type, listener_id, listener)

  self.listeners[listener_id] = listener

  local a = self.by_type

  for i, ev in ipairs(event_types) do
    a[ev] = a[ev] or {}
    a[ev][listener_id] = true
  end

  return listener_id
end

function EventQueue:unlisten(listener_id)
  printf("EVENT::UNLISTEN: listener=%d\n", listener_id)

  table.remove(self.listeners, listener_id)

  for ev_type, listener_ids in pairs(self.by_type) do
    if listener_ids[listener_id] then
      printf("EVENT::UNLISTEN_TYPE: type=%s, listener=%d\n", ev_type, listener_id)
      table.remove(listener_ids, listener_id)
    end
  end
end

function EventQueue:emit(e)
  printf("EVENT::EMIT: event=%s\n", format_table(e))
  local found = false

  queue = self.by_type[e.type]
  if queue then
    if e.listener then
      if queue[e.listener] then
        printf("EVENT::NOTIFY_LISTENER: type=%s, listener=%d\n", e.type, e.listener)
        self.listeners[e.listener](e)
        found = true
      end
    else
      for listener_id, _ in pairs(queue) do
        printf("EVENT::NOTIFY_LISTENER: type=%s, listener=%d\n", e.type, listener_id)
        self.listeners[listener_id](e)
        found = true
      end
    end
  end

  if not found then
    printf("EVENT::NOTIFY_NOT_FOUND: type=%s, listener=%s\n", e.type, e.listener)
  end
end

_G["events"] = EventQueue
