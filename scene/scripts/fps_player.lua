local cmd = self.cmd

local ANIM_IDLE = util.sid("idle")
local ANIM_RUN = util.sid("run")
local ANIM_FIRE = util.sid("fire")

local function attack(wid)
  --print(string.format("START: %d", id))
  return cid;
end

local function animation(self)
  local listener_id
  local wid = 0
  local cid = 0
  local orig_pos = node:get_pos(self.handle)

  local function animation_listener()
    cid = cmd:animation_play(
      { after=wid, sid=ANIM_IDLE } )

    wid = cmd:wait({ after=cid, time=5 })

    cid = cmd:animation_play(
      { after=wid, sid=ANIM_RUN } )

    wid = cmd:wait({ after=cid, time=5 })

    cid = cmd:animation_play(
      { after=wid, speed=0.5, sid=ANIM_FIRE } )

    wid = cmd:wait({ after=cid, time=3 })

    wid = cmd:emit(
      { after=wid },
      { type=Event.SCRIPT_RESUME, listener=listener_id})
  end

  listener_id = self:listen(animation_listener, {Event.SCRIPT_RESUME})

  cmd:emit(
    {},
    { type=Event.SCRIPT_RESUME, listener=listener_id})
end

animation(self)
