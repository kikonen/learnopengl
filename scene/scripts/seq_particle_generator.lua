printf("PARTICLE_GENERATOR: name=%s, clone=%d\n", node:get_name(self.handle), node:get_clone_index(self.handle))

local rnd = math.random

local function animation()
  local wid = 0
  local cid = 0

  local function animation_listener()
    cid = cmd:particle_emit(
      self.handle,
      { after=cid, count=(30 + rnd(50)) * 1000 })

    wid = cmd:wait(
      { after=cid, time=3 })

    cid = cmd:call(
      self.handle,
      { after=wid },
      animation_listener)
  end

  cid = cmd:call(
    self.handle,
    { after=wid },
    animation_listener)
end

animation()
