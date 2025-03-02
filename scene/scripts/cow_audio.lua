--printf("START: name=%s, id=%d, clone=%d\n", node:get_name(), id, node:get_clone_index())

local function animation(coid)
  print("cow run")
  while true do
    local delay = math.random() * 6

    -- printf("moo: delay=%f, id=%i\n", delay, id)

    wid = cmd:wait({ after=cid, time=delay })

    cid = cmd:audio_play(
      { after=wid, sync=true })

    cid = cmd:resume({ after=wid }, coid)
  end
end

print("cow say")
cmd:start({}, animation)
