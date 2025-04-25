local node = self.node
local cmd = self.cmd

local TEXTS = {
  "Happy new year\n" ..
  "\n" ..
  "2025",
  "Have the best\n" ..
  "year coming\n" ..
  "to you!",
  "",
  "Make your\n" ..
  "Promises\n" ..
  "and be kind",
  "With best wishes\n" ..
  "to all",
  ""
}

local function animation()
  local listener_id
  local wid = 0
  local cid = 0
  local idx = 0

  printf("text_start: %s\n", node:get_name())

  local function animation_listener()
    wid = cmd:wait({ after=cid, time=5 })

    printf("text_text: %s\n", TEXTS[idx + 1])

    cid = cmd:set_text({ after=wid, time=5 }, { text=TEXTS[idx + 1] })

    wid = cmd:wait({ after=cid, time=5 })

    if idx == 0 then
      cid = cmd:rotate(
        { after=wid, time=2.5 },
        vec3(0, 1, 0),
        360)
      wid = cmd:wait({ after=cid, time=2.5 })
    end

    cid = cmd:emit(
      { after=wid },
      { type=Event.SCRIPT_RESUME, listener=listener_id})

    idx = (idx + 1) % #TEXTS
  end

  listener_id = events:listen(animation_listener, {Event.SCRIPT_RESUME})

  cid = cmd:set_visible(
    { after=wid },
    false)

  wid = cmd:wait({ after=cid, time=15 })

  cid = cmd:set_visible(
    { after=wid },
    true)

  cid = cmd:emit(
    { after=cid },
    { type=Event.SCRIPT_RESUME, listener=listener_id})
end

--animation()
