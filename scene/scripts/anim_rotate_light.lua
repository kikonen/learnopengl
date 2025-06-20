--printf("START: name=%s, clone=%d\n", node:get_name(), node:get_clone_index())

local function animation()
  local node = self.node
  local cmd = self.cmd

  local wid = 0
  local cid = 0

  local function animation_listener()
    wid = cmd:wait({ after=cid, time=0 })

    cid = cmd:rotate(
      { time=30, relative=true },
      vec3(0.0, 1.0, 0.0),
      360.0)

    wid = cmd:wait({ after=cid, time=0 })

    wid = cmd:call(
      { after=wid },
      animation_listener)
  end

  wid = cmd:call(
    { after=wid },
    animation_listener)
end

animation()
