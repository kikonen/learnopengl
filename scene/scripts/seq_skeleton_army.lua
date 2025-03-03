--printf("START: name=%s, id=%d, clone=%d\n", node:get_name(), id, node:get_clone_index())

local ANIM_IDLE = util.sid("master:Idle")
local ANIM_IDLE_2 = util.sid("master:Idle2")
local ANIM_IDLE_HIT = util.sid("master:Hit")

local ANIM_WALK_1 = util.sid("master:Walk01")
local ANIM_WALK_2 = util.sid("master:Walk02")
local ANIM_RUN = util.sid("master:Run")

local ANIM_SWING_HEAVY = util.sid("master:SwingHeavy")
local ANIM_SWING_NORMAL = util.sid("master:SwingNormal")
local ANIM_SWING_QUICK = util.sid("master:SwingQuick")

local rnd = math.random

local function randomIdle()
  local sid;
  local r = rnd(100);

  if r > 60 then
    sid = ANIM_IDLE
  elseif r > 30 then
    sid = ANIM_IDLE_2
  else
    sid = ANIM_HIT
  end

  return sid;
end

local function randomMove()
  local sid;
  local r = rnd(100);

  if r > 60 then
    sid = ANIM_WALK_1
  elseif r > 30 then
    sid = ANIM_WALK_2
  else
    sid = ANIM_RUN
  end

  return sid;
end

local function randomAttack()
  local sid;
  local r = rnd(100);

  if r > 80 then
    sid = ANIM_SWING_HEAVY
  elseif r > 50 then
    sid = ANIM_SWING_NORMAL
  else
    sid = ANIM_SWING_QUICK
  end

  return sid;
end

local function attack(wid)
  --print(string.format("START: %d", id))

  local pos = node:get_pos()
  local x = pos[1]
  local y = pos[2]
  local z = pos[3]
  local cid = 0

  cid = cmd:animation_play(
    { after=wid, sid=randomMove() } )

  cid = cmd:move(
    { after=wid, time=10 + rnd(5), relative=true },
    { 25 - rnd(100), 0, 25 - rnd(100) })

  wid = cmd:wait({ after=0, time=7 + rnd(5) })
  cmd:cancel({ after=wid, time=0 }, cid)

  cid = cmd:move(
    { after=cid, time=10 + rnd(5), relative=true },
    { 25 - rnd(100), 0, 25 - rnd(100) })

  cmd:cancel({ after=0, time=0 }, cid)

  cid = cmd:animation_play(
    { after=cid, sid=randomAttack() } )

  cid = cmd:move(
    { after=cid, time=5 + rnd(5), relative=true },
    { 25 - rnd(50), 0, 25 - rnd(50) })

  cid = cmd:animation_play(
    { after=cid, sid=randomIdle() } )

  cid = cmd:move(
    { after=cid, time=5 + rnd(5), relative=true },
    { 10 - rnd(20), 0, 10 - rnd(20) })

  cid = cmd:animation_play(
    { after=cid, sid=randomMove() } )

  cid = cmd:move_spline(
    { after=cid, time=3 + rnd(5), relative=true },
    { 20, 0, 5 },
    { 5 - rnd(10), 0, 5 - rnd(10) })

  cid = cmd:animation_play(
    { after=cid, sid=randomAttack() } )

  cid = cmd:move(
    { after=cid, time=2 + rnd(5), relative=false },
    { x, y, z })

  return cid;
end

local function animation()
  local listener_id = nil
  local wid = 0
  local cid = 0
  local orig_pos = nil

  local function animation_listener()
    orig_pos = orig_pos or node:get_pos()

    wid = cmd:wait({ after=cid, time=1 + rnd(5) })

    cid = attack(wid)

    wid = cmd:wait({ after=cid, time=10 + rnd(5) })

    cmd:emit(
      { after=wid },
      { type=Event.SCRIPT_RESUME, listener=listener_id})
  end

  listener_id = events:listen(animation_listener, {Event.SCRIPT_RESUME})

  cmd:emit(
    {},
    { type=Event.SCRIPT_RESUME, listener=listener_id})
end

animation()
