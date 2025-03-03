--printf("START: name=%s, id=%d, clone=%d\n", node:get_name(), id, node:get_clone_index())

local ANIM_IDLE = util:sid("master:Idle")
local ANIM_IDLE_2 = util:sid("master:Idle2")
local ANIM_IDLE_HIT = util:sid("master:Hit")

local ANIM_SWING_HEAVY = util:sid("master:SwingHeavy")
local ANIM_SWING_NORMAL = util:sid("master:SwingNormal")
local ANIM_SWING_QUICK = util:sid("master:SwingQuick")

local rnd = math.random

local function randomIdle()
  local sid
  local r = rnd(10)

  if r > 6 then
    sid = ANIM_IDLE
  elseif r > 2 then
    sid = ANIM_IDLE_2
  else
    sid = ANIM_HIT
  end

  return sid
end

local function randomAttack()
  local sid
  local r = rnd(10)

  if r > 8 then
    sid = ANIM_SWING_HEAVY
  elseif r > 5 then
    sid = ANIM_SWING_NORMAL
  else
    sid = ANIM_SWING_QUICK
  end

  return sid
end

local function idle(wid)
  --print(string.format("idle: %d", id))

  local cid
  cid = cmd:animation_play(
    { after=wid, sid=randomIdle() } )

  return cid
end

local function attack(wid)
  --print(string.format("attack: %d", id))

  local cid

  cid = cmd:animation_play(
    { after=wid, sid=randomAttack() } )

  return cid
end

local function animation()
  local listener_id
  local wid = 0
  local cid = 0

  local function animation_listener()
    cid = idle(wid)
    wid = cmd:wait({ after=cid, time=5 + rnd(10) })

    cid = attack(wid)
    wid = cmd:wait({ after=cid, time=5 + rnd(10) })

    cid = cmd:emit(
      { after=wid },
      { type=Event.SCRIPT_RESUME, listener=listener_id})
  end

  listener_id = events:listen(animation_listener, {Event.SCRIPT_RESUME})

  cmd:emit(
    {},
    { type=Event.SCRIPT_RESUME, listener=listener_id})
end

local function event_test()
  local listener_id

  local function test_listener(e)
    printf("RECEIVED_EVENT: event=%s\n", format_table(e))
  end

  printf("LISTEN_EVENTS: name=%s\n", node:get_name())
  local listener_id = events:listen(test_listener, {"test-1", "test-2"})
  printf("REGISTERED_LISTENER: id=%s\n", listener_id)

  printf("SEND_EVENT: name=%s\n", node:get_name())
  events:emit({type = "test", data = "foo-0"})
  events:emit({type = "test-1", data = "foo-1", listener=listener_id})
  events:emit({type = "test-2", data = "foo-2"})
  events:emit({type = "test-1", data = "foo-1.222", listener=222})
  events:emit({type = "test-3", data = "foo-3"})

  events:unlisten(listener_id)
end

event_test()
animation()
