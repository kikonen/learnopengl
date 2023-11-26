local luaNode = nodes[id]

local rnd = math.random

local function animationTranslate()
  local wid = -1
  local cid = -1

  local center = node:getPos()

  --print(string.format("center: %d, %d, %d", center[1], center[2], center[3]))

  while true do
    -- NOTE KI *WAIT* for resume to complete
    wid = cmd:wait(cid, 0)

    local posX = 10 - rnd(20)
    local posY = 3 - rnd(6)
    local posZ = 5 - rnd(10)

    --print(string.format("move: %d, %d, %d", posX, posY, posZ))

    local pos = { center[1] + posX, center[2] + posY, center[3] + posZ }

    --print(string.format("pos: %d, %d, %d", pos[1], pos[2], pos[3]))

    cid = cmd:moveSpline(
      id,
      { after=wid, time=5, relative=false },
      { rnd(20) - 10 , rnd(10) - 5, rnd(10) - 5},
      pos)

    wid = cmd:wait(cid, 0)
    cid = cmd:resume(id, { after=wid }, "callbackTranslate")

    -- NOTE KI wait for callback
    coroutine.yield()
  end
end

local function animationRotate()
  local wid = -1
  local cid = -1

  local origPos = node:getPos()

  while true do
    -- NOTE KI *WAIT* for resume to complete
    wid = cmd:wait(cid, 0)

    local rotX = 180 - rnd(360)
    local rotY = 180 - rnd(360)
    local rotZ = 180 - rnd(360)

    cid = cmd:rotate(id, { after=wid, time=5, relative=true }, { rotX, rotY, rotZ })

    wid = cmd:wait(cid, 0)
    cid = cmd:resume(id, { after=wid }, "callbackRotate")

    -- NOTE KI wait for callback
    coroutine.yield()
  end
end

local function animationScale()
  local wid = -1
  local cid = -1

  local origPos = node:getPos()

  while true do
    -- NOTE KI *WAIT* for resume to complete
    wid = cmd:wait(cid, 0)

    local scale = rnd(30) / 10.0

    if scale < 0.1 then
      scale = 0.1
    elseif scale > 2.0 then
      scale = 2.0
    end

    cid = cmd:scale(id, { after=wid, time=5, relative=false }, { scale, scale, scale })

    wid = cmd:wait(cid, 0)
    cid = cmd:resume(id, { after=wid }, "callbackScale")

    -- NOTE KI wait for callback
    coroutine.yield()
  end
end

luaNode.start = function()
  --print(string.format("START: name=%s, id=%d, clone=%d", node:getName(), id, node:getCloneIndex()))
  luaNode.callbackTranslate = coroutine.wrap(animationTranslate)
  luaNode.callbackRotate = coroutine.wrap(animationRotate)
  luaNode.callbackScale = coroutine.wrap(animationScale)

  cmd:resume(id, {}, "callbackTranslate")
  cmd:resume(id, {}, "callbackRotate")
  cmd:resume(id, {}, "callbackScale")
end
-- invoked by scene
--luaNode.start()
