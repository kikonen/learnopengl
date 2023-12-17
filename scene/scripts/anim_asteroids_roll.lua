local function animationX(coid)
  local wid = 0
  local cid = 0

  while true do
    wid = cmd:wait({ after=cid, time=0 })

    cid = cmd:rotate(
      { after=wid, time=120, relative=true },
      { 1.0, 0.0, 0.0 },
      180.0)

    wid = cmd:wait({ after=cid, time=0 })

    cid = cmd:resume({ after=wid }, coid)
  end
end

local function animationY(coid)
  local wid = 0
  local cid = 0

  while true do
    wid = cmd:wait({ after=cid, time=0 })

    cid = cmd:rotate(
      { after=wid, time=100, relative=true },
      { 0.0, 1.0, 0.0 },
      180.0)

    wid = cmd:wait({ after=cid, time=0 })

    cid = cmd:resume({ after=wid }, coid)
  end
end

local function animationZ(coid)
  local wid = 0
  local cid = 0

  while true do
    wid = cmd:wait({ after=cid, time=0 })

    cid = cmd:rotate(
      { after=wid, time=140, relative=true },
      { 0.0, 0.0, 1.0 },
      180.0)

    wid = cmd:wait({ after=cid, time=0 })

    cid = cmd:resume({ after=wid }, coid)
  end
end

cmd:start({}, animationX)
cmd:start({}, animationY)
cmd:start({}, animationZ)
