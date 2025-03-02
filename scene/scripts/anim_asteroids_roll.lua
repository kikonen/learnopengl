local function animation_x()
  local listener_id = nil
  local orig_pos = node:get_pos()
  local wid = 0
  local cid = 0

  local function animation_listener()
    wid = cmd:wait({ after=cid, time=0 })

    cid = cmd:rotate(
      { after=wid, time=120, relative=true },
      { 1.0, 0.0, 0.0 },
      180.0)

    wid = cmd:wait({ after=cid, time=0 })

    cid = cmd:emit(
      { after=wid },
      { type=Event.SCRIPT_RESUME, listener=listener_id})
  end

  listener_id = events:listen(animation_listener, {Event.SCRIPT_RESUME})

  cmd:emit(
    {},
    { type=Event.SCRIPT_RESUME, listener=listener_id})
end

local function animation_y()
  local listener_id = nil
  local orig_pos = node:get_pos()
  local wid = 0
  local cid = 0

  local function animation_listener()
    wid = cmd:wait({ after=cid, time=0 })

    cid = cmd:rotate(
      { after=wid, time=100, relative=true },
      { 0.0, 1.0, 0.0 },
      180.0)

    wid = cmd:wait({ after=cid, time=0 })

    cid = cmd:emit(
      { after=wid },
      { type=Event.SCRIPT_RESUME, listener=listener_id})
  end

  listener_id = events:listen(animation_listener, {Event.SCRIPT_RESUME})

  cmd:emit(
    {},
    { type=Event.SCRIPT_RESUME, listener=listener_id})
end

local function animation_z()
  local listener_id = nil
  local orig_pos = node:get_pos()
  local wid = 0
  local cid = 0

  local function animation_listener()
    wid = cmd:wait({ after=cid, time=0 })

    cid = cmd:rotate(
      { after=wid, time=140, relative=true },
      { 0.0, 0.0, 1.0 },
      180.0)

    wid = cmd:wait({ after=cid, time=0 })

    cid = cmd:emit(
      { after=wid },
      { type=Event.SCRIPT_RESUME, listener=listener_id})
  end

  listener_id = events:listen(animation_listener, {Event.SCRIPT_RESUME})

  cmd:emit(
    {},
    { type=Event.SCRIPT_RESUME, listener=listener_id})
end

animation_x()
animation_y()
animation_z()
