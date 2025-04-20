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
  local rotate_cid = 0
  local move_cid = 0
  local attack_cid = 0

  local cast_cid = 0
  local ray_degrees = 0
  local elapsed = 0

  local function ray_cast_callback(args)
    if not args.data.is_hit then
      -- print("NO_HIT")
      return
    end

    if elapsed < 1 then
      return
    end

    elapsed = 0

    print("RAY_HIT")
    table_print(args)

    printf("front: %s\n", node:get_front())

    local n1 = node:get_front()
    local n2 = args.data.pos - node:get_pos()
    n1.y = 0
    n2.y = 0
    n1 = n1:normalize()
    n2 = n2:normalize()

    local cosine = glm.dot(n1, n2)
    local angle = glm.degrees(math.acos(cosine))

    printf("n1: %s\n", n1)
    printf("n2: %s\n", n2)
    printf("cosine: %f\n", cosine)

    if cosine < 0 then
      angle = angle - 180
    end

    local rot_degrees = util.degrees_between(
      n1,
      n2)
    -- local rot = util.axis_degrees_to_quat(vec3(0, 1, 0), rot_degrees)

    printf("rotate: %f\n", rot_degrees)
    printf("angle: %f\n", angle)
    printf("ray_degrees: %f\n", ray_degrees)

    cmd:particle_emit(
      { count=(10 + rnd(50)) * 100 })

    local cancel_cid = cmd:cancel(
      {},
      { rotate_cid, move_sid, attack_sid })

    rotate_cid = cmd:rotate(
      { after=cancel_cid, time=1, relative=true },
      vec3(0, 1, 0),
      rot_degrees)

    move_cid = cmd:move(
      { after=cancel_cid, time=5, relative=false },
      args.data.pos)

    attack_cid = attack(cancel_cid)

    ray_degrees = 0
  end

  local function cast_update(dt)
    elapsed = elapsed + dt

    local rot = util.axis_degrees_to_quat(vec3(0, 1, 0), ray_degrees)
    local dir = rot:to_mat4() * node:get_front()

    ray_degrees = ray_degrees + dt * 3.5

    cast_cid = cmd:cancel(
      {},
      { cast_cid })

    cast_cid = cmd:ray_cast(
      { after=cast_cid },
      { dir },
      ray_cast_callback)
  end

  Updater:add_updater(cast_update)
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

local function start()
  animation()
  ray_caster()
end

local function update(dt)
  -- printf("skeleton_update: dt=%f\n", dt or 0)
end

Updater:add_updater(update)
start()
