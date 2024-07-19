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

local function idle(wid)
  --print(string.format("idle: %d", id))

  local cid;
  cid = cmd:animationPlay(
    { after=wid, name = randomIdle() } )

  return cid;
end

local function attack(wid)
  --print(string.format("attack: %d", id))

  local cid;

  cid = cmd:animationPlay(
    { after=wid, name = randomAttack() } )

  return cid;
end

local function animation(coid)
  local wid = 0
  local cid = 0

  while true do
    --print(string.format("loop: %d", id))

    cid = idle(wid)
    wid = cmd:wait({ after=cid, time=2 + rnd(6) })

    cid = attack(wid)
    wid = cmd:wait({ after=cid, time=1 + rnd(4) })

    cid = cmd:resume({ after=cid }, coid)
  end
end

cmd:start({}, animation)
