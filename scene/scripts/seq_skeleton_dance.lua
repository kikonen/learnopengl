--printf("START: name=%s, id=%d, clone=%d", node:get_name(), id, node:get_clone_index())

local rnd = math.random

local function randomIdle()
  local name;
  local r = rnd(100);

  if r > 60 then
    name = "master:Idle"
  elseif r > 30 then
    name = "master:Idle2"
  else
    name = "master:Hit2"
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

  local cid = 0

  cid = cmd:animation_play(
    { after=wid, name = randomMove() } )

  wid = cmd:wait({ after=0, time=3 + rnd(5) })
  cmd:cancel({ after=wid, time=0 }, cid)

  cmd:cancel({ after=0, time=0 }, cid)

  cid = cmd:animation_play(
    { after=cid, name = randomMove() } )

  cid = cmd:animation_play(
    { after=cid, name = randomAttack() } )

  cid = cmd:animation_play(
    { after=cid, name = randomIdle() } )

  cid = cmd:animation_play(
    { after=cid, name = randomIdle() } )

  return cid;
end

local function animation(coid)
  local wid = 0
  local cid = 0

  while true do
    wid = cmd:wait({ after=cid, time=1 + rnd(3) })

    cid = attack(wid)

    wid = cmd:wait({ after=cid, time=4 + rnd(3) })

    cid = cmd:resume({ after=wid }, coid)
  end
end

cmd:start({}, animation)
