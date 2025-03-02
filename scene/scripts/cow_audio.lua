--printf("START: name=%s, id=%d, clone=%d\n", node:get_name(), id, node:get_clone_index())

local moo_sid = util:sid("moo")

local function animation()
  local listener_id
  local wid = 0
  local cid = 0
  local orig_pos = node:get_pos()

  print("cow run")

  local function animation_listener()
    local delay = math.random() * 6

    -- printf("moo: delay=%f, id=%i\n", delay, id)

    wid = cmd:wait({ after=cid, time=delay })

    cid = cmd:audio_play(
      { after=wid, sync=true, sid=moo_sid })

    cid = cmd:emit(
      { after=cid },
      { type=Event.SCRIPT_RESUME, listener=listener_id})
  end

  listener_id = events:listen(animation_listener, {Event.SCRIPT_RESUME})

  cmd:emit(
    {},
    { type=Event.SCRIPT_RESUME, listener=listener_id})
end

print("cow say")
animation()
