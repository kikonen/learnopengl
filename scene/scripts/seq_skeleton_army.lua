local function main()
  --print(string.format("START: %d", id))

  local pos = node:getPos()
  local rnd = math.random
  local x = pos[1]
  local y = pos[2]
  local z = pos[3]
  local wid = -1
  local cid = -1

  wid = cmd:wait(0, 5)
  cid = cmd:move(id, { after=wid, time=10, relative=true }, { 25 - rnd(100), 0, 25 - rnd(100) })

  wid = cmd:wait(0, 7)
  cmd:cancel(wid, 0, cid)

  cid = cmd:move(id, { after=cid, time=10, relative=true }, { 25 - rnd(100), 0, 25 - rnd(100) })
  cmd:cancel(-1, 0, cid)

  cid = cmd:move(id, { after=cid, time=5, relative=true }, { 25 - rnd(50), 0, 25 - rnd(50) })
  cid = cmd:move(id, { after=cid, time=5, relative=true }, { 10 - rnd(20), 0, 10 - rnd(20) })
  cid = cmd:moveSpline(id, { after=cid, time=3, relative=true }, { 20, 0, 5 }, { 5 - rnd(10), 0, 5 - rnd(10) })
  cid = cmd:move(id, { after=cid, time=2, relative=false }, { x, y, z })
end

cmd:start(id, { after=wid }, main)
--main()
