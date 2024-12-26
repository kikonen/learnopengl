local wid = 0
local cid = 0

wid = cmd:wait({ after=0, time=5 })

cid = cmd:move_spline(
  { after=wid, time=20, relative=false },
  { -50, 50, 0 },
  { -20, 50, 0 })

wid = cmd:wait({ after=cid, time=60 })

cmd:move_spline(
  { after=wid, time=20, relative=true },
  { 50, -40, 0 },
  { 20, -40, 0 })
