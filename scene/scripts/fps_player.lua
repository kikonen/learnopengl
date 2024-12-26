--printf("START: name=%s, id=%d, clone=%d", node:get_name(), id, node:get_clone_index())

local function attack(wid)
  --print(string.format("START: %d", id))
  return cid;
end

local function handler(coid)
  local wid = 0
  local cid = 0

  local orig_pos = node:get_pos()

  while true do
    cid = cmd:animation_play(
      { after=wid, name = "idle:Unreal Take" } )

    wid = cmd:wait({ after=cid, time=5 })

    cid = cmd:animation_play(
      { after=wid, name = "run:Unreal Take" } )

    wid = cmd:wait({ after=cid, time=5 })

    cid = cmd:animation_play(
      { after=wid, speed=0.5, name = "fire:Unreal Take" } )

    wid = cmd:wait({ after=cid, time=3 })

    cid = cmd:resume({ after=wid }, coid)
  end
end

cmd:start({}, handler)
