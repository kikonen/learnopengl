local function animation(coid)
  while true do
    local delay = math.random() * 10

    printf("moo: delay=%f, id=%i\n", delay, id)

    wid = cmd:wait({ after=cid, time=delay })

    cid = cmd:audioPlay(
      { after=wid, sync=true })

    cid = cmd:resume({ after=wid }, coid)
  end
end

cmd:start({}, animation)
