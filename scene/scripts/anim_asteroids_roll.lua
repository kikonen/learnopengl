local luaNode = nodes[id]

local function animation()
   local wid = -1
   local cid = -1
   local cid1 = -1
   local cid2 = -1
   local cid3 = -1

   local origPos = node:getPos()

   while(true) do
      -- NOTE KI *WAIT* for resume to complete
      wid = cmd:wait(cid, 0)

      cid1 = cmd:rotate(id, { after=wid, time=30, relative=true }, { 0.0, 180.0, 0.0 })
      cid2 = cmd:rotate(id, { after=wid, time=20, relative=true }, { 180.0, 0, 0.0 })
      cid3 = cmd:rotate(id, { after=wid, time=30, relative=true }, { 0, 0, 180.0 })

      cid = cmd:sync({ after=-1 }, { cid1, cid2, cid3 })
      wid = cmd:wait(cid, 0)
      cid = cmd:resume(id, { after=wid }, "callback")

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
