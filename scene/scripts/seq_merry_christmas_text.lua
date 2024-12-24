local TEXTS = {
  "Christmas\n" ..
    "    Time\n",
    "Time of relax\n" ..
    "  for full day",
  "   Merry\n" ..
  "Christmas\n" ..
  "   to All!",
  "With best wishes\n" ..
  "from the land of snow",
}

local function animateText(coid)
  local wid = 0
  local cid = 0
  local idx = 0

  wid = cmd:wait({ after=cid, time=5 })

  while true do
    wid = cmd:wait({ after=cid, time=5 })

    printf("text text: %s", TEXTS[idx + 1])

    cid = cmd:set_text({ after=wid, time=5 }, { text=TEXTS[idx + 1] })
    idx = (idx + 1) % #TEXTS

    wid = cmd:wait({ after=cid, time=5 })

    cid = cmd:resume({ after=wid }, coid)
  end
end

cmd:start({}, animateText)
