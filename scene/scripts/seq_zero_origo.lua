local wid = -1
local cid = -1

wid = cmd:wait(0, 5)
cid = cmd:moveSpline(id, { after=wid, time=20, relative=false }, { -50, 50, 0 }, { -20, 50, 0 })

wid = cmd:wait(cid, 60)
cmd:moveSpline(id, { after=wid, time=20, relative=true }, { 50, -40, 0 }, { 20, -40, 0 })
