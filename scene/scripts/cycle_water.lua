
local function animation()
   local wid = -1
   local cid = -1

   local origPos = node:getPos()

   while(true) do
      wid = cmd:wait(cid, 3)
      cid = cmd:moveSpline(
         id,
         { after=wid, time=10, relative=true },
         { 0, 1.5, 0 },
         { 0, -2.5, 0 })

      wid = cmd:wait(cid, 1)
      cid = cmd:moveSpline(
         id,
         { after=wid, time=10, relative=false },
         { 0, -1.5, 0 },
         origPos)

      wid = cmd:wait(cid, 1)
      cmd:resume(id, { after=wid }, "callback")

      -- NOTE KI wait for callback
      coroutine.yield()
   end
end

local function start()
   print(string.format("START-water: %d", id))

   local luaNode = nodes[id]
   local wid = -1

   luaNode.callback = coroutine.wrap(animation)

   cmd:resume(id, { after=wid }, "callback")
end

start()
