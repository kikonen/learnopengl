local function animation()
  local orig_pos = node:get_pos(self.handle)
  local wid = 0
  local cid = 0
  local dir = 1

  local function animation_listener()
    wid = cmd:wait({ after=cid, time=0.5 })

    cid = cmd:rotate(
      self.handle,
      { after=wid, time=3, relative=true },
      vec3(0, 1, 0),
      dir * 45)

    cid = cmd:call(
      self.handle,
      { after=cid },
      animation_listener)

    dir = -dir
  end

  wid = cmd:wait({ after=cid, time=1 })

  cid = cmd:call(
    self.handle,
    { after=wid },
    animation_listener)
end

animation()
