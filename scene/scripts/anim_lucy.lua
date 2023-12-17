printf("START: name=%s, id=%d, clone=%d", node:getName(), id, node:getCloneIndex())

local function animation(coid)
  local wid = 0
  local cid = 0
  local dir = 1

  local AUDIO_WIND = 0
  local AUDIO_ROTATE = 1

  -- prepause
  cid = cmd:wait({ after=cid, time=1 })

  while true do
    wid = cmd:wait({ after=cid, time=0.5 })

    cid = cmd:audioPlay(
      { after=cid, index=AUDIO_ROTATE })

    cid = cmd:rotate(
      { after=wid, time=20, relative=true },
      { 0, 1, 0 },
      dir * 720)

    cid = cmd:audioPause(
      { after=cid, index=AUDIO_ROTATE })

    cid = cmd:audioPlay(
      { after=cid, index=AUDIO_WIND, sync=true })

    cid = cmd:resume({ after=cid }, coid)
    dir = -dir
  end
end

local function animation_2(coid)
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

cmd:start({}, animation)
--  cmd:start({}, animation_2)
