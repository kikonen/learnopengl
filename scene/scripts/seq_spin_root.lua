local wid = 0
local cid = 0

wid = cmd:wait({ after=0, time=10 })

cmd:rotate(
  { after=wid, time=10, relative=true },
  vec3(0.0, 1.0, 0.0),
  360.0)
