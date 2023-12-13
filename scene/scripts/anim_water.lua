local luaNode = nodes[id]

local function animation(coid)
  local wid = 0
  local cid = 0

  local origPos = node:getPos()

  while true do
    wid = cmd:wait({ after=cid, time=3 })

    cid = cmd:moveSpline(
      { after=wid, time=10, relative=true },
      { 0, 1.5, 0 },
      { 0, -2.5, 0 })

    wid = cmd:wait({ after=cid, time=1 })

    cid = cmd:moveSpline(
      { after=wid, time=10, relative=false },
      { 0, -1.5, 0 },
      origPos)

    wid = cmd:wait({ after=cid, time=1 })

    cid = cmd:resume({ after=wid }, coid)
  end
end

luaNode.start = function()
  --print(string.format("START: name=%s, id=%d, clone=%d", node:getName(), id, node:getCloneIndex()))

  cmd:start({}, animation)
end
