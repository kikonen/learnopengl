local luaNode = nodes[id]

local function animation(coid)
  local wid = -1
  local cid = -1

  local origPos = node:getPos()

  while true do
    -- NOTE KI *WAIT* for resume to complete
    wid = cmd:wait(cid, 3)

    cid = cmd:moveSpline(
      0,
      { after=wid, time=10, relative=true },
      { 0, 1.5, 0 },
      { 0, -2.5, 0 })

    wid = cmd:wait(cid, 1)
    cid = cmd:moveSpline(
      0,
      { after=wid, time=10, relative=false },
      { 0, -1.5, 0 },
      origPos)

    wid = cmd:wait(cid, 1)
    cid = cmd:resume(coid, { after=wid }, "_")
  end
end

luaNode.start = function()
  --print(string.format("START: name=%s, id=%d, clone=%d", node:getName(), id, node:getCloneIndex()))
  --luaNode.callback = coroutine.wrap(animation)
  --cmd:resume(id, {}, "callback")
  cmd:start(0, {}, animation)
end
