printf("PARTICLE_GENERATOR: name=%s, id=%d, clone=%d\n", node:get_name(), id, node:get_clone_index())

local rnd = math.random

local function animation(coid)
  local cid = 0
  local wid = 0

  while true do
    cid = cmd:particle_emit(
      { after=wid, count=(30 + rnd(50)) * 100 })

    wid = cmd:wait(
      { after=cid, time=3 })

    cid = cmd:resume({ after=wid }, coid)
  end
end

cmd:start({}, animation)
