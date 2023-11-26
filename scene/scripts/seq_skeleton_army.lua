local luaNode = nodes[id]

local function attack(wid)
  --print(string.format("START: %d", id))

  local pos = node:getPos()
  local rnd = math.random
  local x = pos[1]
  local y = pos[2]
  local z = pos[3]
  local cid = 0

  cid = cmd:move(
    { after=wid, time=10, relative=true },
    { 25 - rnd(100), 0, 25 - rnd(100) })

  wid = cmd:wait({ after=0, time=7 })
  cmd:cancel({ after=wid, time=0 }, cid)

  cid = cmd:move(
    { after=cid, time=10, relative=true },
    { 25 - rnd(100), 0, 25 - rnd(100) })

  cmd:cancel({ after=0, time=0 }, cid)

  cid = cmd:move(
    { after=cid, time=5, relative=true },
    { 25 - rnd(50), 0, 25 - rnd(50) })

  cid = cmd:move(
    { after=cid, time=5, relative=true },
    { 10 - rnd(20), 0, 10 - rnd(20) })

  cid = cmd:moveSpline(
    { after=cid, time=3, relative=true },
    { 20, 0, 5 },
    { 5 - rnd(10), 0, 5 - rnd(10) })

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

luaNode.start = function()
  --print(string.format("START: name=%s, id=%d, clone=%d", node:getName(), id, node:getCloneIndex()))

  cmd:start({}, animation)
end
