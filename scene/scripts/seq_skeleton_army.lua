--printf("START: name=%s, id=%d, clone=%d\n", node:get_name(), id, node:get_clone_index())

local rnd = math.random

local function randomIdle()
  local name;
  local r = rnd(100);

  if r > 60 then
    name = "master:Idle"
  elseif r > 30 then
    name = "master:Idle2"
  else
    name = "master:Hit"
  end

  return name;
end

local function randomMove()
  local name;
  local r = rnd(100);

  if r > 60 then
    name = "master:Walk01"
  elseif r > 30 then
    name = "master:Walk02"
  else
    name = "master:Run"
  end

  return name;
end

local function randomAttack()
  local name;
  local r = rnd(100);

  if r > 80 then
    name = "master:SwingHeavy"
  elseif r > 50 then
    name = "master:SwingNormal"
  else
    name = "master:SwingQuick"
  end

  return name;
end

local function attack(wid)
  --print(string.format("START: %d", id))

  local pos = node:get_pos()
  local x = pos[1]
  local y = pos[2]
  local z = pos[3]
  local cid = 0

  cid = cmd:animation_play(
    { after=wid, name = randomMove() } )

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
    { after=cid, name = randomAttack() } )

  cid = cmd:move(
    { after=cid, time=5 + rnd(5), relative=true },
    { 25 - rnd(50), 0, 25 - rnd(50) })

  cid = cmd:animation_play(
    { after=cid, name = randomIdle() } )

  cid = cmd:move(
    { after=cid, time=5 + rnd(5), relative=true },
    { 10 - rnd(20), 0, 10 - rnd(20) })

  cid = cmd:animation_play(
    { after=cid, name = randomMove() } )

  cid = cmd:move_spline(
    { after=cid, time=3 + rnd(5), relative=true },
    { 20, 0, 5 },
    { 5 - rnd(10), 0, 5 - rnd(10) })

  cid = cmd:animation_play(
    { after=cid, name = randomAttack() } )

  cid = cmd:move(
    { after=cid, time=2 + rnd(5), relative=false },
    { x, y, z })

  return cid;
end

local function animation(coid)
  local wid = 0
  local cid = 0

  local orig_pos = node:get_pos()

  while true do
    wid = cmd:wait({ after=cid, time=1 + rnd(5) })

    cid = attack(wid)

    wid = cmd:wait({ after=cid, time=10 + rnd(5) })

    cid = cmd:resume({ after=wid }, coid)
  end
end

cmd:start({}, animation)
