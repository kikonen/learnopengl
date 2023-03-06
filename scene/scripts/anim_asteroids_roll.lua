local luaNode = nodes[id]

local function animationX()
   local wid = -1
   local cid = -1

   while(true) do
      -- NOTE KI *WAIT* for resume to complete
      wid = cmd:wait(cid, 0)

      cid = cmd:rotate(id, { after=wid, time=120, relative=true }, { 180.0, 0, 0.0 })

      wid = cmd:wait(cid, 0)
      cid = cmd:resume(id, { after=wid }, "callback")

      -- NOTE KI wait for callback
      coroutine.yield()
   end
end

local function animationY()
   local wid = -1
   local cid = -1

   while(true) do
      -- NOTE KI *WAIT* for resume to complete
      wid = cmd:wait(cid, 0)

      cid = cmd:rotate(id, { after=wid, time=100, relative=true }, { 0.0, 180.0, 0.0 })

      wid = cmd:wait(cid, 0)
      cid = cmd:resume(id, { after=wid }, "callback")

      -- NOTE KI wait for callback
      coroutine.yield()
   end
end

local function animationZ()
   local wid = -1
   local cid = -1

   while(true) do
      -- NOTE KI *WAIT* for resume to complete
      wid = cmd:wait(cid, 0)

      cid = cmd:rotate(id, { after=wid, time=140, relative=true }, { 0, 0, 180.0 })

      wid = cmd:wait(cid, 0)
      cid = cmd:resume(id, { after=wid }, "callback")

      -- NOTE KI wait for callback
      coroutine.yield()
   end
end

luaNode.start = function()
   --print(string.format("START: name=%s, id=%d, clone=%d", node:getName(), id, node:getCloneIndex()))
   luaNode.callbackX = coroutine.wrap(animationX)
   luaNode.callbackY = coroutine.wrap(animationY)
   luaNode.callbackZ = coroutine.wrap(animationZ)

   cmd:resume(id, {}, "callbackX")
   cmd:resume(id, {}, "callbackY")
   cmd:resume(id, {}, "callbackZ")
end
-- invoked by scene
--luaNode.start()
