--printf("START: name=%s, id=%d, clone=%d", node:get_name(), id, node:get_clone_index())

local function animation_rotate(coid)
  local wid = 0
  local cid = 0
  local dir = 1

  local AUDIO_WIND = util:sid("wind")
  local AUDIO_ROTATE = util:sid("rotate")

  -- prepause
  cid = cmd:wait({ after=cid, time=1 })

  while true do
    wid = cmd:wait({ after=cid, time=0.5 })

    cid = cmd:audio_play(
      { after=cid, sid=AUDIO_ROTATE })

    cid = cmd:rotate(
      { after=wid, time=40, relative=true },
      { 0, 1, 0 },
      dir * 720 * 2)

    cid = cmd:audio_pause(
      { after=cid, sid=AUDIO_ROTATE })

    cid = cmd:audio_play(
      { after=cid, sid=AUDIO_WIND, sync=true })

    cid = cmd:resume({ after=cid }, coid)
    dir = -dir
  end
end

local function animation_scale(coid)
  local wid = 0
  local cid = 0
  local dir = 1

  while true do
    wid = cmd:wait({ after=cid, time=1 })

    local scale = { 1, 1, 1 }
    if dir < 0 then
      scale = { 4, 4, 4 }
    end

    cid = cmd:scale(
      { after=wid, time=20, relative=false },
      scale)

    cid = cmd:resume({ after=cid }, coid)
    dir = -dir
  end
end

cmd:start({}, animation_rotate)
cmd:start({}, animation_scale)
