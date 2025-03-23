--printf("START: name=%s, id=%d, clone=%d\n", node:get_name(), id, node:get_clone_index())
nodes[id].node = node
nodes[id].cmd = cmd
n = nodes[id]

local ANIM_IDLE = util.sid("master:Idle")
local ANIM_IDLE_2 = util.sid("master:Idle2")
local ANIM_HIT = util.sid("master:Hit")

local ANIM_SWING_HEAVY = util.sid("master:SwingHeavy")
local ANIM_SWING_NORMAL = util.sid("master:SwingNormal")
local ANIM_SWING_QUICK = util.sid("master:SwingQuick")

local EXPLODE_SID = util.sid("explode")

printf("LUA: SID=%d, SID_NAME=%s\n", ANIM_SWING_QUICK, util.sid_name(ANIM_SWING_QUICK))

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

function lua_node:explode()
  explode_cid = cmd:audio_play(
    { sync=true, sid=EXPLODE_SID })
end

function lua_node:emit_particles()
  cmd:particle_emit(
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

    cid = cmd:rotate(
      { time=1, relative=true },
      vec3(0, 1, 0), degrees)

    cid = attack(cid)

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

local function animation()
  local idx = 0
  local wid = 0
  local cid = 0
  local cid2 = 0
  local pos = node:get_pos()

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

    wid = cmd:wait({ after=cid, time=5 + rnd(10) })

    wid = cmd:call(
      { after=wid },
      animation_listener)

    idx = idx + 1
  end

  wid = cmd:call(
    { after=wid },
    animation_listener)
end

animation()
ray_caster()
