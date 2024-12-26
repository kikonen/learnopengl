--printf("START: name=%s, id=%d, clone=%d", node:get_name(), id, node:get_clone_index())

local rnd = math.random

local function animationTranslate(coid)
  local wid = 0
  local cid = 0

  local center = node:get_pos()

  --print(string.format("center: %d, %d, %d", center[1], center[2], center[3]))

  while true do
    wid = cmd:wait({ after=cid, time=0 })

    local posX = 10 - rnd(20)
    local posY = 3 - rnd(6)
    local posZ = 5 - rnd(10)

    --print(string.format("move: %d, %d, %d", posX, posY, posZ))

    local pos = { center[1] + posX, center[2] + posY, center[3] + posZ }

    --print(string.format("pos: %d, %d, %d", pos[1], pos[2], pos[3]))

    cid = cmd:move_spline(
      { after=wid, time=5, relative=false },
      { rnd(20) - 10 , rnd(10) - 5, rnd(10) - 5},
      pos)

    wid = cmd:wait({ after=cid, time=0 })

    cid = cmd:resume({ after=wid }, coid)
  end
end

local function animationRotate(coid)
  local wid = 0
  local cid = 0

  local orig_pos = node:get_pos()

  while true do
    wid = cmd:wait({ after=cid, time=0 })

    local rotX = 180 - rnd(360)
    local rotY = 180 - rnd(360)
    local rotZ = 180 - rnd(360)

    cid1 = cmd:rotate(
      { after=wid, time=5, relative=true },
      { 1, 0, 0},
      rotX)

    cid2 = cmd:rotate(
      { after=wid, time=5, relative=true },
      { 0, 1, 0},
      rotY)

    cid3 = cmd:rotate(
      { after=wid, time=5, relative=true },
      { 0, 0, 1 },
      rotZ)

    cid = cmd:sync(
      { after=wid },
      { cid1, cid2, cid3 })

    wid = cmd:wait({ after=cid, time=0 })

    cid = cmd:resume({ after=wid }, coid)
  end
end

local function animationScale(coid)
  local wid = 0
  local cid = 0

  local orig_pos = node:get_pos()

  while true do
    wid = cmd:wait({ after=cid, time=0 })

    local scale = rnd(30) / 10.0

    if scale < 0.1 then
      scale = 0.1
    elseif scale > 2.0 then
      scale = 2.0
    end

    cid = cmd:scale(
      { after=wid, time=5, relative=false },
      { scale, scale, scale })

    wid = cmd:wait({ after=cid, time=0 })

    cid = cmd:resume({ after=wid }, coid)
  end
end

cmd:start({}, animationTranslate)
cmd:start({}, animationRotate)
cmd:start({}, animationScale)
