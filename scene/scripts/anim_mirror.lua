--printf("START: name=%s, id=%d, clone=%d\n", node:get_name(), id, node:get_clone_index())

local function animation(coid)
  local wid = 0
  local cid = 0

  local orig_pos = node:get_pos()

  while true do
    wid = cmd:wait({ after=cid, time=1 })

    -- slow for frontside of mirror
    cid = cmd:rotate(
      { after=wid, time=120, relative=true },
      { 0.0, 1.0, 0.0 },
      120.0)

    -- fast for backside of mirror
    cid = cmd:rotate(
      { after=cid, time=8, relative=true },
      { 0.0, 1.0, 0.0 },
      240.0)

    wid = cmd:wait({ after=cid, time=1 })

    cid = cmd:resume({ after=wid }, coid)
  end
end

cmd:start({}, animation)
