local node = self.node
local cmd = self.cmd

local TEXTS = {
  "This the story\n" ..
    "And it will continue\n" ..
    "So be prepared\n" ..
    "until the end",
  "Viva la vida!",
  "Terveisiä täältä! - ÄÅÖ - äåö",
}

local function animation()
  local listener_id = nil
  local orig_pos = node:get_pos()
  local wid = 0
  local cid = 0
  local idx = 0

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

  listener_id = events:listen(animation_listener, {Event.SCRIPT_RESUME})

  cmd:emit(
    {},
    { type=Event.SCRIPT_RESUME, listener=listener_id})
end

animation()
