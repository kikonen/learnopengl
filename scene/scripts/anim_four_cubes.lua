local luaNode = nodes[id]

local cloneIndex = node:getCloneIndex()
local origPos = node:getPos()

local function animationMove()
   local wid = -1
   local cid = -1

   local dir = 1
   if cloneIndex % 2 == 0 then
      dir = -1
   end

   local speed = 30

   while(true) do
      -- NOTE KI *WAIT* for resume to complete
      wid = cmd:wait(cid, 2)
      cid = cmd:move(id, { after=wid, time=speed, relative=true }, {dir * 10, 0, 0.0 })
      cid = cmd:resume(id, { after=cid }, "callbackMove")
      dir = -1 * dir

      -- NOTE KI wait for callback
      coroutine.yield()
   end
end

local function animationRotation()
   local wid = -1
   local cid = -1

   local dir = 1
   if cloneIndex % 2 == 0 then
      dir = -1
   end

   local speed = 120 / ((cloneIndex + 1) * 0.5)

   while(true) do
      -- NOTE KI *WAIT* for resume to complete
      wid = cmd:wait(cid, 2)
      cid = cmd:rotate(id, { after=wid, time=speed, relative=true }, {0.0, dir * 360, 0.0 })
      cid = cmd:resume(id, { after=cid }, "callback")
      dir = -1 * dir

      -- NOTE KI wait for callback
      coroutine.yield()
   end
end

luaNode.start = function()
   --print(string.format("START: name=%s, id=%d, clone=%d", node:getName(), id, node:getCloneIndex()))
   luaNode.callbackMove = coroutine.wrap(animationMove)
   luaNode.callbackRotation = coroutine.wrap(animationRotation)

   cmd:resume(id, {}, "callbackMove")
   cmd:resume(id, {}, "callbackRotation")
end
-- invoked by scene
--luaNode.start()
