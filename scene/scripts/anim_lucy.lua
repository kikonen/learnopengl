local luaNode = nodes[id]

local function animation()
  local wid = -1
  local cid = -1
  local dir = 1

  wid = cmd:wait(cid, 1)

  while true do
    -- NOTE KI *WAIT* for resume to complete
    wid = cmd:wait(wid, 0.5)

    cid = cmd:rotate(
      id,
      { after=wid, time=10, relative=true },
      { 0, dir * 360, 0 })

    cid = cmd:resume(id, { after=cid }, "callback")
    wid = cid
    dir = -dir

    -- NOTE KI wait for callback
    coroutine.yield()
  end
end

luaNode.start = function()
  --print(string.format("START: name=%s, id=%d, clone=%d", node:getName(), id, node:getCloneIndex()))
  luaNode.callback = coroutine.wrap(animation)
  cmd:resume(id, {}, "callback")
end
-- invoked by scene
--luaNode.start()
