local node = self.node
local cmd = self.cmd

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

    cid = cmd:set_text({ after=wid, time=5 }, { text=TEXTS[idx + 1] })
    idx = (idx + 1) % #TEXTS

    wid = cmd:wait({ after=cid, time=5 })

    cid = cmd:emit(
      { after=wid },
      { type=Event.SCRIPT_RESUME, listener=listener_id})
  end

  listener_id = self:listen(animation_listener, {Event.SCRIPT_RESUME})

  cmd:emit(
    {},
    { type=Event.SCRIPT_RESUME, listener=listener_id})
end

animation(self)
