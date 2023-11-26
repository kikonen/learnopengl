local luaNode = nodes[id]

local cloneIndex = node:getCloneIndex()
local origPos = node:getPos()

local function animationMove(coid)
  local wid = 0
  local cid = 0

  local dir = 1
  if cloneIndex % 2 == 0 then
    dir = -1
  end

  local speed = 30

  while true do
    wid = cmd:wait({ after=cid, time=2 })

    cid = cmd:move(
      { after=wid, time=speed, relative=true },
      {dir * 10, 0, 0.0 })

    cid = cmd:resume({ after=cid }, coid)
    dir = -1 * dir
  end
end

local function animationRotate(coid)
  local wid = 0
  local cid = 0

  local dir = 1
  if cloneIndex % 2 == 0 then
    dir = -1
  end

  local speed = 120 / ((cloneIndex + 1) * 0.5)

  while true do
    wid = cmd:wait({ after=cid, time=2 })

    cid = cmd:rotate(
      { after=wid, time=speed, relative=true },
      {0.0, dir * 360, 0.0 })

    cid = cmd:resume({ after=cid }, coid)
    dir = -1 * dir
  end
end

luaNode.start = function()
  --print(string.format("START: name=%s, id=%d, clone=%d", node:getName(), id, node:getCloneIndex()))

  cmd:start({}, animationMove)
  cmd:start({}, animationRotate)
end
