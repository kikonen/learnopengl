printf("EMIT\n")

local shoot_sid = util:sid("shoot")
local explode_sid = util:sid("explode")

local function animation(coid)
  local orig_pos = node:get_pos()

  local cid = 0;
  local wid = 0;

  cid = cmd:wait(
    { after=cid, time=10 })

  while true do
    cid = cmd:move(
      { after=cid, time=0 },
      orig_pos)

    cid = cmd:set_visible(
      { after=cid },
      true)

    wid = cmd:wait(
      { after=cid, time=0.5 })

    cid = cmd:audio_play(
      { after=wid, sync=true, sid=shoot_sid })

    cid = cmd:move(
      { after=wid, time=3 },
      {0, 20, 0})

    wid = cmd:wait(
      { after=cid, time = 0.2 })

    cid = cmd:set_visible(
      { after=wid },
      false)

    cid = cmd:particle_emit(
      { after=wid, count=50 * 1000 })

    cid = cmd:audio_play(
      { after=cid, sync=true, sid=explode_sid })

    cid = cmd:resume({ after=cid }, coid)
  end
end

cmd:start({}, animation)
