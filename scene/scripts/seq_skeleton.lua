--printf("START: name=%s, id=%d, clone=%d", node:get_name(), id, node:get_clone_index())

local rnd = math.random

local function randomIdle()
  local name;
  local r = rnd(10);

  if r > 6 then
    name = "master:Idle"
  elseif r > 2 then
    name = "master:Idle2"
  else
    name = "master:Hit"
  end

  return name;
end

local function randomAttack()
  local name;
  local r = rnd(10);

  if r > 8 then
    name = "master:SwingHeavy"
  elseif r > 5 then
    name = "master:SwingNormal"
  else
    name = "master:SwingQuick"
  end

  return name;
end

local function idle(wid)
  --print(string.format("idle: %d", id))

  local cid;
  cid = cmd:animation_play(
    { after=wid, name = randomIdle() } )

  return cid;
end

local function attack(wid)
  --print(string.format("attack: %d", id))

  local cid;

  cid = cmd:animation_play(
    { after=wid, name = randomAttack() } )

  return cid;
end

local function animation(coid)
  local wid = 0
  local cid = 0

  while true do
    --print(string.format("loop: %d", id))

    cid = idle(wid)
    wid = cmd:wait({ after=cid, time=5 + rnd(10) })

    cid = attack(wid)
    wid = cmd:wait({ after=cid, time=5 + rnd(10) })

    cid = cmd:resume({ after=cid }, coid)
  end
end

cmd:start({}, animation)
