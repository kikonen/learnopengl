local node = self.node
local cmd = self.cmd

--printf("START: name=%s, clone=%d\n", node:get_name(), node:get_clone_index())

local ANIM_IDLE = util.sid("master:Idle")
local ANIM_IDLE_2 = util.sid("master:Idle2")
local ANIM_HIT = util.sid("master:Hit")

local ANIM_SWING_HEAVY = util.sid("master:SwingHeavy")
local ANIM_SWING_NORMAL = util.sid("master:SwingNormal")
local ANIM_SWING_QUICK = util.sid("master:SwingQuick")

local EXPLODE_SID = util.sid("explode")

printf("LUA: SID=%d, SID_NAME=%s\n", ANIM_SWING_QUICK, util.sid_name(ANIM_SWING_QUICK))

local rnd = math.random

function State:ditto(args)
  print("LUA: DITTO_SHARED_CALLED")
  printf("LUA: self=%s\n", table_format(self));
  printf("LUA: args=%s\n", table_format(args));
end

local function ditto(args)
  print("LUA: DITTO_LOCAL_CALLED")
  printf("LUA: self=%s\n", table_format(self));
  printf("LUA: args=%s\n", table_format(args));
end

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

function State:explode()
  explode_cid = self.cmd:audio_play(
    { sync=true, sid=EXPLODE_SID })
end

function State:emit_particles()
  self.cmd:particle_emit(
    { count=(10 + rnd(50)) * 1000 })
end

local function ray_caster()
  local cid = 0
  local degrees = 0

  local function ray_cast_hit(args)
    print("RAY_HIT")
    table_print(args)

    cmd:particle_emit(
      { count=(10 + rnd(50)) * 1000 })

    cmd:rotate({ time=1, relative=true }, vec3(0, 1, 0), degrees)
    degrees = 0;
  end

  local function ray_cast()
    cid = cmd:wait({ after=cid, time=0.25 })

    local rot = util.axis_degrees_to_quat(vec3(0, 1, 0), degrees)
    printf("front=%s, rot=%s\n", node:get_front(), rot)
    local dir = rot:to_mat4() * node:get_front()
    printf("dir=%s\n", dir)

    cid = cmd:ray_cast(
      { after=cid },
      dir,
      false,
      ray_cast_hit)

    cid = cmd:wait({ after=cid, time=0.25 })

    cid = cmd:call(
      { after=cid },
      ray_cast)

    degrees = degrees + 1.5
  end

  cid = cmd:call(
    { after=cid },
    ray_cast)
end

local function animation(self)
  -- local listener_id
  local idx = 0
  local wid = 0
  local cid = 0
  local pos = node:get_pos()

  printf("node=%s\n", node:str())
  printf("pos=%s = %f\n", pos, pos:length())

  local v2 = vec3(1.0, 2.0, 3.0)
  printf("v2=%s = %f\n", v2, v2:length())

  local n2 = v2:normalize()
  printf("n2=%s = %f\n", n2, n2:length())

  local m3 = mat3(1)
  local v3 = m3 * v2
  printf("m3=%s, v3=%s = %f\n", m3, v3, v3:length())

  local m4 = mat4(2)
  local v4 = m4 * vec4(v2, 1)
  printf("m4=%s, v4=%s = %f\n", m4, v4, v4:length())

  local v5 = vec3(v4)
  printf("v5=%s = %f\n", v5, v5:length())

  local m6 = node:get_model_matrix()
  local v6 = m6 * vec4(1, 0, 0, 1)
  printf("m6=%s, v6=%s = %f\n", m6, v6, v6:length())

  local v7_1 = vec3(1, 0, 0)
  local v7_2 = vec3(0, 1, 0)
  local v7_3 = glm.cross(v7_1, v7_2)
  printf("v7_1=%s, v7_2=%s, v7_3=%s\n", v7_1, v7_2, v7_3)

  local function animation_listener()
    cid = idle(wid)

    if idx == 0 then
      cid2 = cmd:move(
        { after=wid, time=5, relative=true },
        vec3(-2, 0, 0))

      cid2 = cmd:move(
        { after=wid, time=5, relative=true },
        vec3(0, 0, 2))
    end

    cid = cmd:call(
      { after=cid, object=true },
      self.ditto,
      { bar="start_of_seq_self" })

    cid = cmd:call(
      { after=cid },
      ditto,
      { bar="start_of_seq_local" })

    wid = cmd:wait({ after=cid, time=5 + rnd(10) })

    cid = attack(wid)
    wid = cmd:wait({ after=cid, time=5 + rnd(10) })

    wid = cmd:call(
      { after=wid, object=true },
      self.ditto,
      { ditto="end_of_seq_self" })

    -- wid = cmd:emit(
    --   { after=wid },
    --   { type=Event.SCRIPT_RESUME, listener=listener_id})

    wid = cmd:call(
      { after=wid },
      animation_listener)

    idx = idx + 1
  end

  -- listener_id = self:listen(animation_listener, {Event.SCRIPT_RESUME})

  -- cmd:emit(
  --   {},
  --   { type=Event.SCRIPT_RESUME, listener=listener_id})

  wid = cmd:call(
    { after=wid },
    animation_listener)
end

local function event_test(self)
  local listener_id

  local function test_listener(e)
    printf("RECEIVED_EVENT: event=%s\n", table_format(e))
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

event_test(self)
animation(self)
ray_caster(self)
