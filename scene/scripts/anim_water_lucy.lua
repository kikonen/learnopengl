--printf("START: name=%s, id=%d, clone=%d\n", node:get_name(), id, node:get_clone_index())

local function animation(coid)
  local wid = 0
  local cid = 0

  local orig_pos = node:get_pos()

  while true do
    wid = cmd:wait({ after=cid, time=10 })

    cid = cmd:move_spline(
      { after=wid, time=10, relative=true },
      { 0, 1.5, 0 },
      { 0, -2.5, 0 })

    wid = cmd:wait({ after=cid, time=1 })
    cid = cmd:move_spline(
      { after=wid, time=10, relative=false },
      { 0, -1.5, 0 },
      orig_pos)

    wid = cmd:wait({ after=cid, time=1 })

    cid = cmd:resume({ after=wid }, coid)
  end
end

cmd:start({}, animation)
