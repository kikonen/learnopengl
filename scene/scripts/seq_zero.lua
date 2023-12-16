local wid = 0
local cid = 0

wid = cmd:wait({ after=0, time=5 })

cmd:scale(
  { after=wid, time=40, relative=true },
  { 4.0, 6.0, 0.2 })

wid = cmd:wait({ after=0, time=30 })

cid = cmd:rotate(
  { after=wid, time=10, relative=true },
  { 0.0, 1.0, 0.0 },
  720.0)

wid = cmd:wait({ after=cid, time=30 })

cmd:rotate(
  { after=wid, time=10, relative=true },
  { 0.0, 1.0, 0.0 },
  720.0)

cmd:scale(
  { after=wid, time=20, relative=false },
  { 1.2, 1.2, 0.2 })
