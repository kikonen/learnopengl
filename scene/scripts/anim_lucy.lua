
local function animation()
   local wid = -1
   local cid = -1

   wid = cmd:wait(cid, 1)

   while(true) do
      cid = cmd:rotate(
         id,
         { after=wid, time=10, relative=true },
         { 0, 360, 0 })
      wid = -1

      cmd:resume(id, { after=cid }, "callback")

      -- NOTE KI wait for callback
      coroutine.yield()
   end
end

local function start()
   local luaNode = nodes[id]
   local wid = -1

   luaNode.callback = coroutine.wrap(animation)

   cmd:resume(id, { after=wid }, "callback")
end

start()
