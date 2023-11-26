local luaNode = nodes[id]

local function animation()
  local wid = -1
  local cid = -1

  local origPos = node:getPos()

  dir = 1
  scale = 600

  while true do
    -- NOTE KI *WAIT* for resume to complete
    wid = cmd:wait(cid, 1)

    cid = cmd:move(id, { after=wid, time=30, relative=true }, { dir * scale, 0.0, 0.0 })

    wid = cmd:wait(cid, 1)
    cid = cmd:resume(id, { after=wid }, "callback")

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
