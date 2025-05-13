local node = self.node
local cmd = self.cmd

--printf("START: name=%s, clone=%d\n", node:get_name(), node:get_clone_index())

local ANIM_IDLE = util.sid("master:Idle")
local ANIM_IDLE_2 = util.sid("master:Idle2")
local ANIM_HIT = util.sid("master:Hit")

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

  local cid = 0

  cid = cmd:animation_play(
    { after=wid, sid=randomMove() } )

  wid = cmd:wait({ after=0, time=3 + rnd(5) })
  cmd:cancel({ after=wid, time=0 }, cid)

  cmd:cancel({ after=0, time=0 }, cid)

  cid = cmd:animation_play(
    { after=cid, sid=randomMove() } )

  cid = cmd:animation_play(
    { after=cid, sid=randomAttack() } )

  cid = cmd:animation_play(
    { after=cid, sid=randomIdle() } )

  cid = cmd:animation_play(
    { after=cid, sid=randomIdle() } )

  return cid;
end

local function animation()
  local listener_id
  local wid = 0
  local cid = 0

  local function animation_listener()
    wid = cmd:wait({ after=cid, time=1 + rnd(3) })

    cid = attack(wid)

    wid = cmd:wait({ after=cid, time=4 + rnd(3) })

    cid = cmd:emit(
      { after=wid },
      { type=Event.SCRIPT_RESUME, listener=listener_id})
  end

  listener_id = events:listen(animation_listener, {Event.SCRIPT_RESUME})

  cmd:emit(
    {},
    { type=Event.SCRIPT_RESUME, listener=listener_id})
end

animation()
