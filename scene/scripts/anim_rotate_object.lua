--printf("START: name=%s, id=%d, clone=%d", node:get_name(), id, node:get_clone_index())

local function animation(coid)
  local wid = 0
  local cid = 0
  local dir = 1

  wid = cmd:wait({ after=cid, time=1 })

  while true do
    wid = cmd:wait({ after=cid, time=0.5 })

    cid = cmd:rotate(
      { after=wid, time=3, relative=true },
      { 0, 1, 0 },
      dir * 45)


    cid = cmd:resume({ after=cid }, coid)
    dir = -dir
  end
end

cmd:start({}, animation)
