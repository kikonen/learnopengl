local luaNode = nodes[id]

local function animation(coid)
  local wid = 0
  local cid = 0

  local origPos = node:getPos()

  dir = 1
  scale = 600

  while true do
    wid = cmd:wait({ after=cid, time=1 })

    cid = cmd:move(
      { after=wid, time=30, relative=true },
      { dir * scale, 0.0, 0.0 })

    wid = cmd:wait({ after=cid, time=1 })

    cid = cmd:resume({ after=wid }, coid)
    dir = -dir
  end
end

luaNode.start = function()
  --print(string.format("START: name=%s, id=%d, clone=%d", node:getName(), id, node:getCloneIndex()))

  cmd:start({}, animation)
end
