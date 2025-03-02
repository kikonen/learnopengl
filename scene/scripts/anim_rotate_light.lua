--printf("START: name=%s, id=%d, clone=%d\n", node:get_name(), id, node:get_clone_index())

local function animation(coid)
  local wid = 0
  local cid = 0

  local orig_pos = node:get_pos()

  while true do
    wid = cmd:wait({ after=cid, time=0 })

    cid = cmd:rotate(
      { time=30, relative=true },
      { 0.0, 1.0, 0.0 },
      360.0)

    wid = cmd:wait({ after=cid, time=0 })

    cid = cmd:resume({ after=wid }, coid)
  end
end

cmd:start({}, animation)
