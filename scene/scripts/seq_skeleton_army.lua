--printf("START: name=%s, id=%d, clone=%d", node:getName(), id, node:getCloneIndex())

local rnd = math.random

local function randomIdle()
  local name;
  local r = rnd(10);

  if r > 6 then
    name = "Idle"
  elseif r > 2 then
    name = "Idle2"
  else
    name = "Hit"
  end

  return name;
end

local function randomMove()
  local name;
  local r = rnd(10);

  if r > 6 then
    name = "Walk01"
  elseif r > 2 then
    name = "Walk02"
  else
    name = "Run"
  end

  return name;
end

local function randomAttack()
  local name;
  local r = rnd(10);

  if r > 8 then
    name = "SwingHeavy"
  elseif r > 5 then
    name = "SwingNormal"
  else
    name = "SwingQuick"
  end

  return name;
end

local function attack(wid)
  --print(string.format("START: %d", id))

  local pos = node:getPos()
  local x = pos[1]
  local y = pos[2]
  local z = pos[3]
  local cid = 0

  cid = cmd:animationPlay(
    { after=wid, name = randomMove() } )

  cid = cmd:move(
    { after=wid, time=10, relative=true },
    { 25 - rnd(100), 0, 25 - rnd(100) })

  wid = cmd:wait({ after=0, time=7 })
  cmd:cancel({ after=wid, time=0 }, cid)

  cid = cmd:move(
    { after=cid, time=10, relative=true },
    { 25 - rnd(100), 0, 25 - rnd(100) })

  cmd:cancel({ after=0, time=0 }, cid)

  cid = cmd:animationPlay(
    { after=cid, name = randomAttack() } )

  cid = cmd:move(
    { after=cid, time=5, relative=true },
    { 25 - rnd(50), 0, 25 - rnd(50) })

  cid = cmd:animationPlay(
    { after=cid, name = randomIdle() } )

  cid = cmd:move(
    { after=cid, time=5, relative=true },
    { 10 - rnd(20), 0, 10 - rnd(20) })

  cid = cmd:animationPlay(
    { after=cid, name = randomMove() } )

  cid = cmd:moveSpline(
    { after=cid, time=3, relative=true },
    { 20, 0, 5 },
    { 5 - rnd(10), 0, 5 - rnd(10) })

  cid = cmd:animationPlay(
    { after=cid, name = randomAttack() } )

  cid = cmd:move(
    { after=cid, time=2, relative=false },
    { x, y, z })

  return cid;
end

local function animation(coid)
  local wid = 0
  local cid = 0

  local origPos = node:getPos()

  while true do
    wid = cmd:wait({ after=cid, time=1 })

    cid = attack(wid)

    wid = cmd:wait({ after=cid, time=10 })

    cid = cmd:resume({ after=wid }, coid)
  end
end

cmd:start({}, animation)
