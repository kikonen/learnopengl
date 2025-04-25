local node = self.node
local cmd = self.cmd

printf("PARTICLE_GENERATOR: name=%s, clone=%d\n", node:get_name(), node:get_clone_index())

local rnd = math.random

local function animation()
  local wid = 0
  local cid = 0

  local function animation_listener()
    cid = cmd:particle_emit(
      { after=cid, count=(30 + rnd(50)) * 100 })

    wid = cmd:wait(
      { after=cid, time=3 })

    cid = cmd:call(
      { after=wid },
      animation_listener)
  end

  cid = cmd:call(
    { after=wid },
    animation_listener)
end

animation()
