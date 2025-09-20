printf("EMIT\n")

local rnd = math.random

local shoot_sid = util.sid("shoot")
local explode_sid = util.sid("explode")

local function animation(self)
  local listener_id
  local cid = 0
  local cid2 = 0
  local explode_cid = 0
  local wid = 0
  local wid2 = 0
  local orig_pos = node:get_pos(self.handle)

  local function animation_listener()
    cid = cmd:move(
      self.handle,
      { after=cid, time=0 },
      orig_pos)

    cid = cmd:set_visible(
      self.handle,
      { after=cid },
      true)

    cmd:set_visible(
      self.text_node_handle,
      { after=cid },
      false)

    cid = cmd:set_text(
      self.text_node_handle,
      { after=cid },
      { text="" })

    wid = cmd:wait(
      { after=cid, time=0.5 })

    cid = cmd:audio_play(
      self.handle,
      { after=wid, sync=true, sid=shoot_sid })

    cid = cmd:move(
      self.handle,
      { after=wid, time=2 + rnd(100)/100.0, relative=true },
      vec3(0, 10 + rnd(10), 0))

    wid = cmd:wait(
      { after=cid, time = 0.2 })

    cmd:set_visible(
      self.handle,
      { after=wid },
      false)

    cid2 = cmd:particle_emit(
      self.handle,
      { after=wid, count=(30 + rnd(50)) * 1000 })

    explode_cid = cmd:audio_play(
      self.handle,
      { after=cid2, sync=true, sid=explode_sid })

    wid2 = cmd:wait(
      { after=cid2, time = 1 })

    cmd:set_visible(
      self.text_node_handle,
      { after=wid2 },
      true)

    cid = cmd:set_text(
      self.text_node_handle,
      { after=wid2 },
      { text="Happy new year!!!" })

    wid = cmd:wait(
      { after=cid2, time=0.3 })

    cid = cmd:rotate(
      self.text_node_handle,
      { after=wid, time=1 },
      vec3(0, 1, 0),
      360)

    wid = cmd:wait(
      { after=cid, time=0.3 })

    cid = cmd:emit(
      self.handle,
      { after=wid },
      { type=Event.SCRIPT_RESUME, listener=listener_id})
  end

  listener_id = self:listen(animation_listener, {Event.SCRIPT_RESUME})

  cid = cmd:wait(
    { after=cid, time=8 + rnd(5) })

  cid = cmd:emit(
    self.handle,
    { after=cid },
    { type=Event.SCRIPT_RESUME, listener=listener_id})
end

if not State.initialize then
(function()
  print("Register STATE: START")
  function State:initialize()
    self.text_node_handle = scene:find_node({ tag="rocket_message" })
    debug("text_node=%s\n", self.text_node_handle)

    animation(self)
  end
end)()
else
  print("Register STATE: ALREADY_DONE")
end
