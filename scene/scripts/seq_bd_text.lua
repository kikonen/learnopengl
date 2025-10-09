local TEXTS = {
  "Birthday\n" ..
    "it is\n",
    "So enjoy\n" ..
    "until the end of day",
  "Feliz cumpleaños!",
  "Wishes from here\n" ..
  "other side of the world",
}

local function animation(self)
  local listener_id
  local wid = 0
  local cid = 0
  local idx = 0

  cid = cmd:wait({ after=cid, time=5 })

  local function animation_listener()
    wid = cmd:wait({ after=cid, time=5 })

    printf("text text: %s\n", TEXTS[idx + 1])

    cid = cmd:set_text(
      self.handle,
      { after=wid, time=5 }, { text=TEXTS[idx + 1] })

    idx = (idx + 1) % #TEXTS

    wid = cmd:wait({ after=cid, time=5 })

    cid = cmd:emit(
      self.handle,
      { after=wid },
      { type=Event.SCRIPT_RESUME, listener=listener_id})
  end

  listener_id = self:listen(animation_listener, {Event.SCRIPT_RESUME})

  cmd:emit(
    self.handle,
    {},
    { type=Event.SCRIPT_RESUME, listener=listener_id})
end

animation(self)
