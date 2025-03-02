--printf("START: name=%s, id=%d, clone=%d\n", node:get_name(), id, node:get_clone_index())

local function animation(coid)
  local wid = 0
  local cid = 0
  local dir = 1

  local count = 0
  local angle = 5
  local time = 3

  while true do
    wid = cmd:wait({ after=cid, time=0.5 })

    cid = cmd:rotate(
      { after=wid, time=time, relative=true },
      { 0, 0, 1 },
      dir * angle )

    cid = cmd:resume({ after=cid }, coid)

    if count == 0 then
      angle = angle * 2
      time = time * 2
    end
    dir = -dir
    count = count + 1
  end
end

cmd:start({}, animation)
