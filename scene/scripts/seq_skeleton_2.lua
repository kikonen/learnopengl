--printf("START: name=%s, clone=%d\n", node:get_name(), node:get_clone_index())

local rnd = math.random

if not State.initialize then
(function()
  debug("Register STATE")

  local ANIM_IDLE = SID("idle")
  local ANIM_IDLE_2 = SID("idle_2")
  local ANIM_HIT = SID("hit")

local ANIM_SWING_HEAVY = SID("swing_heavy")
local ANIM_SWING_NORMAL = SID("swing_normal")
local ANIM_SWING_QUICK = SID("swing_quick")

  local EXPLODE_SID = SID("explode")

  -- debug("LUA: SID=%d, SID_NAME=%s\n", ANIM_SWING_QUICK, SID_NAME(ANIM_SWING_QUICK))

  function State:initialize()
    self.body_id = self.node:find_child({ tag = SID("body") })
  end

  function State:explode()
    explode_cid = self.cmd:audio_play(
      { sync=true, sid=EXPLODE_SID })
  end

  function State:emit_particles()
    self.cmd:particle_emit(
      { count=(10 + rnd(50)) * 1000 })
  end

  function State:random_idle()
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

  function State:random_attack()
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

  function State:idle(wid)
    --print(string.format("idle: %d", id))

    local cid
    cid = self.cmd:animation_play(
      { after=wid, node = self.body_id, sid=self:random_idle() } )

    return cid
  end

  function State:attack(wid)
    --print(string.format("attack: %d", id))

    local cid

    cid = self.cmd:animation_play(
      { after=wid, node = self.body_id, sid=self:random_attack() } )

    return cid
  end

  print("-------------")
  table_print(self)
  print("-------------")
end)()
else
  print("Register STATE: ALREADY_DONE")
end

local INITIAL_RAY_DEGREES = 50 - rnd(100)

local function ray_caster(self)
  local node = self.node
  local cmd = self.cmd

  local rotate_cid = 0
  local move_cid = 0
  local path_cid = 0
  local attack_cid = 0

  local cast_cid = 0
  local ray_degrees = INITIAL_RAY_DEGREES
  local elapsed = 5
  local cast_elapsed = 1

  local function path_callback(args)
    print("NAV: PATH")
    table_print(args)

    local node_front = node:get_front()
    local node_pos = node:get_pos()

    debug("front: %s\n", node_front)

    local prev_pos = node_pos
    local prev_cid = 0

    for i = 0, args.data:len() - 1 do
      local next_pos = args.data:waypoint(i)
      local rel_pos = next_pos - prev_pos

      debug("A: waypoint-%d: %s, rel=%s\n", i, next_pos, rel_pos)

      attack_cid = self:attack(prev_cid)

      prev_cid = cmd:move(
        { after=prev_cid, time=0.5, relative=true },
        rel_pos)

      prev_pos = next_pos
    end

    -- prev_cid = cmd:move_path(
    --   { after=prev_cid, time=4, relative=false },
    --   args.data.waypoints)
  end

  local function ray_cast_callback(args)
    debug("ray_cast_callback: %s\n", table_format(args))
    if not args.data.is_hit then
      -- print("NO_HIT")
      return
    end

    debug("RAY_HIT: elapsed=%f\n", elapsed)

    if elapsed < 5 then
      return
    end

    elapsed = 0

    print("RAY_HIT")
    table_print(args)

    cmd:particle_emit(
      { count=(10 + rnd(50)) * 100 })

    debug("front: %s\n", node:get_front())

    local nodePos = node:get_pos()
    local targetPos = args.data.pos
    local n1 = node:get_front()
    local n2 = targetPos - nodePos
    n1.y = 0
    n2.y = 0
    n1 = n1:normalize()
    n2 = n2:normalize()

    local cosine = glm.dot(n1, n2)
    local angle = glm.degrees(math.acos(cosine))

    debug("n1: %s\n", n1)
    debug("n2: %s\n", n2)
    debug("cosine: %f\n", cosine)

    if cosine < 0 then
      angle = angle - 180
    end

    local rot_degrees = util.degrees_between(
      n1,
      n2)
    -- local rot = util.axis_degrees_to_quat(vec3(0, 1, 0), rot_degrees)

    debug("rotate: %f\n", rot_degrees)
    debug("angle: %f\n", angle)
    debug("ray_degrees: %f\n", ray_degrees)

    local cancel_cid = 0

    cancel_cid = cmd:cancel(
      {},
      rotate_cid)

    cancel_cid = cmd:cancel(
      {},
      move_cid)

    cancel_cid = cmd:cancel(
      {},
      attack_cid)

    rotate_cid = cmd:rotate(
      { after=cancel_cid, time=1, relative=true },
      vec3(0, 1, 0),
      rot_degrees)

    -- move_cid = cmd:move(
    --   { after=cancel_cid, time=5, relative=false },
    --   targetPos)

    -- attack_cid = self:attack(cancel_cid)

    ray_degrees = 0

    path_cid = cmd:find_path(
      { after=cancel_cid, time=0 },
      nodePos,
      targetPos,
      --nodePos + vec3(0, 0, -10),
      100,
      path_callback)
  end

  local function cast_update(dt)
    elapsed = elapsed + dt
    cast_elapsed = cast_elapsed + dt

    -- NOTE KI avoid busy lopping in ray cast
    if cast_elapsed < 1 then return end

    ray_degrees = ray_degrees - cast_elapsed * 1
    cast_elapsed = 0

    local rot_quat = util.axis_degrees_to_quat(vec3(0, 1, 0), ray_degrees)
    local rot_degrees = rot_quat:to_degrees()

    local dir = (rot_quat:to_mat4() * node:get_front()):normalize()
    -- local dir_quat = util.normal_to_quat(dir, vec3(0, 1, 0))
    -- local dir_degrees = dir_quat:to_degrees()

    debug("CAST[%d] rot=%s, dir=%s\n",
      self.id, rot_degrees, dir)

    cast_cid = cmd:cancel(
      {},
      cast_cid)

    cast_cid = cmd:ray_cast(
      { after=cast_cid },
      dir,
      false,
      ray_cast_callback)
  end

  self:add_updater(cast_update)
end

local function animation(self)
  local node = self.node
  local cmd = self.cmd

  local idx = 0
  local wid = 0
  local cid = 0
  local cid2 = 0
  local pos = node:get_pos()

  local function animation_listener()
    cid = self:idle(wid)

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

local function start(self)
  animation(self)
  ray_caster(self)
end

start(self)
