local node = self.node
local cmd = self.cmd

printf("EMIT\n")

local rnd = math.random

local shoot_sid = util.sid("shoot")
local explode_sid = util.sid("explode")
-- local text_node_handle = scne:find_node({ tag="rocket_message" })

local function animation(self)
  local listener_id
  local cid = 0
  local cid2 = 0
  local explode_cid = 0
  local wid = 0
  local wid2 = 0
  local orig_pos = node:get_pos()

  local function animation_listener()
    cid = cmd:move(
      { after=cid, time=0 },
      orig_pos)

    cid = cmd:set_visible(
      { after=cid },
      true)

    -- cmd:set_visible(
    --   { after=cid, node=text_node_handle },
    --   false)

    -- cid = cmd:set_text({ after=cid, node=text_node_handle },
    --   { text="" })

    wid = cmd:wait(
      { after=cid, time=0.5 })

    cid = cmd:audio_play(
      { after=wid, sync=true, sid=shoot_sid })

    cid = cmd:move(
      { after=wid, time=2 + rnd(100)/100.0, relative=true },
      vec3(0, 10 + rnd(10), 0))

    wid = cmd:wait(
      { after=cid, time = 0.2 })

    cmd:set_visible(
      { after=wid },
      false)

    cid2 = cmd:particle_emit(
      { after=wid, count=(30 + rnd(50)) * 1000 })

    explode_cid = cmd:audio_play(
      { after=cid2, sync=true, sid=explode_sid })

    -- wid2 = cmd:wait(
    --   { after=cid2, time = 1 })

    -- cmd:set_visible(
    --   { after=wid2, node=text_node_handle },
    --   true)

    -- cid = cmd:set_text(
    --   { after=wid2, node=text_node_handle },
    --   { text="Happy new year!!!" })

    wid = cmd:wait(
      { after=cid2, time=0.3 })

    -- cid = cmd:rotate(
    --   { after=wid, node=text_node_handle, time=1 },
    --   vec3(0, 1, 0),
    --   360)

    -- wid = cmd:wait(
    --   { after=cid, time=0.3 })

    cid = cmd:emit(
      { after=wid },
      { type=Event.SCRIPT_RESUME, listener=listener_id})
  end

  listener_id = self:listen(animation_listener, {Event.SCRIPT_RESUME})

  cid = cmd:wait(
    { after=cid, time=8 + rnd(5) })

  cid = cmd:emit(
    { after=cid },
    { type=Event.SCRIPT_RESUME, listener=listener_id})
end

animation(self)
