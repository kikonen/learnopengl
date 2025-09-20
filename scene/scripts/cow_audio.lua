local moo_sid = util.sid("moo")

local function animation(self)
  local listener_id
  local wid = 0
  local cid = 0
  local orig_pos = node:get_pos(self.handle)

  print("cow run")

  local function animation_listener()
    local delay = math.random() * 6

    wid = cmd:wait({ after=cid, time=delay })

    cid = cmd:audio_play(
      self.handle,
      { after=wid, sync=true, sid=moo_sid })

    cid = cmd:emit(
      self.handle,
      { after=cid },
      { type=Event.SCRIPT_RESUME, listener=listener_id})
  end

  listener_id = self:listen(animation_listener, {Event.SCRIPT_RESUME})

  cmd:emit(
    self.handle,
    {},
    { type=Event.SCRIPT_RESUME, listener=listener_id})
end

print("cow say")
animation(self)
