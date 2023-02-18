local wid = -1
local cid = -1

wid = cmd:wait(-1, 10)
cmd:rotate(id, { after=wid, time=10, relative=true }, { 0.0, 360.0, 0.0 })
