local TEXTS = {
  "Happy new year\n",
  "Have the best\n" ..
  "year coming\n" ..
  "to you!",
  "",
  "Make your\n" ..
  "Promises\n" ..
  "and be kind",
  "With best wishes\n" ..
  "to all",
}

local function animateText(coid)
  local wid = 0
  local cid = 0
  local idx = 0

  printf("text_start: %s\n", node:get_name())

  cmd:set_visible(
    { after=wid },
    false)

  wid = cmd:wait({ after=cid, time=10 })

  cmd:set_visible(
    { after=wid },
    true)

  while true do
    wid = cmd:wait({ after=cid, time=5 })

    printf("text_text: %s\n", TEXTS[idx + 1])

    cid = cmd:set_text({ after=wid, time=5 }, { text=TEXTS[idx + 1] })

    wid = cmd:wait({ after=cid, time=5 })

    if idx == 0 then
      cid = cmd:rotate(
        { after=wid, time=2 },
        { 0, 1, 0 },
        360)
      wid = cmd:wait({ after=cid, time=1.5 })
    end

    cid = cmd:resume({ after=wid }, coid)

    idx = (idx + 1) % #TEXTS
  end
end

cmd:start({}, animateText)
