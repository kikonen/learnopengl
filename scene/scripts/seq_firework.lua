printf("EMIT\n")

local rnd = math.random

local shoot_sid = util:sid("shoot")
local explode_sid = util:sid("explode")
local text_node_sid = util:sid("rocket_message")

local function animation(coid)
  local orig_pos = node:get_pos()

  local cid = 0
  local cid2 = 0
  local explode_cid = 0
  local wid = 0
  local wid2 = 0

  cid = cmd:wait(
    { after=cid, time=5 + rnd(5) })

  while true do
    cid = cmd:move(
      { after=cid, time=0 },
      orig_pos)

    cid = cmd:set_visible(
      { after=cid },
      true)

    cmd:set_visible(
      { after=cid, node=text_node_sid },
      false)

    cid = cmd:set_text({ after=cid, node=text_node_sid },
      { text="" })

    wid = cmd:wait(
      { after=cid, time=0.5 })

    cid = cmd:audio_play(
      { after=wid, sync=true, sid=shoot_sid })

    cid = cmd:move(
      { after=wid, time=2 + rnd(100)/100.0, relative=true },
      {0, 10 + rnd(10), 0})

    wid = cmd:wait(
      { after=cid, time = 0.2 })

    cmd:set_visible(
      { after=wid },
      false)

    cid2 = cmd:particle_emit(
      { after=wid, count=(30 + rnd(50)) * 1000 })

    explode_cid = cmd:audio_play(
      { after=cid2, sync=true, sid=explode_sid })

    wid2 = cmd:wait(
      { after=cid2, time = 1 })

    cmd:set_visible(
      { after=wid2, node=text_node_sid },
      true)

    cid = cmd:set_text(
      { after=wid2, node=text_node_sid },
      { text="Happy new year!!!" })

    wid = cmd:wait(
      { after=cid, time=0.3 })

    cid = cmd:rotate(
      { after=wid, node=text_node_sid, time=1 },
      { 0, 1, 0 },
      360)

    wid = cmd:wait(
      { after=cid, time=0.3 })

    cid = cmd:resume({ after=wid }, coid)
  end
end

cmd:start({}, animation)
