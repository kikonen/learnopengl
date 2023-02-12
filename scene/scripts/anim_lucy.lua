
local function animation()
   local wid = -1
   local cid = -1
   local dir = 1

   wid = cmd:wait(cid, 1)

   while(true) do
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

local function start()
   local luaNode = nodes[id]
   local wid = -1

   luaNode.callback = coroutine.wrap(animation)

   cmd:resume(id, { after=wid }, "callback")
end

start()
