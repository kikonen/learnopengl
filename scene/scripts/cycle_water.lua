function cycleWater()
   local wid = -1
   local cid = -1

   wid = cmd:wait(cid, 3)
   cid = cmd:moveSpline(id, { after=wid, time=10, relative=true }, { 2, 2, 0 }, { 0, -2, 0 })
   wid = cmd:wait(cid, 1)
   cmd:moveSpline(id, { after=wid, time=10, relative=true }, { 3,- 1, 0 }, { 0, 2.1, 0 })
end

--coroutine.create(cycleWater)
--coroutine.resume(cycleWater)
--cmd::start(cycleWater)

cycleWater()
