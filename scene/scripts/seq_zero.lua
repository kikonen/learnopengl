local wid = -1
local cid = -1

wid = cmd:wait(0, 5)
cmd:scale(id, { after=wid, time=40, relative=true }, { 4.0, 6.0, 0.2 })

wid = cmd:wait(0, 30)
cid = cmd:rotate(id, { after=wid, time=10, relative=true }, { 0.0, 720.0, 0.0 })

wid = cmd:wait(cid, 30)
cmd:rotate(id, { after=wid, time=10, relative=true }, { 0.0, 720.0, 0.0 })
cmd:scale(id, { after=wid, time=20, relative=false }, { 1.2, 1.2, 0.2 })
