local function animation(coid)
  print("cow run")
  while true do
    local delay = math.random() * 60

    printf("moo: delay=%f, id=%i\n", delay, id)

    wid = cmd:wait({ after=cid, time=delay })

    cid = cmd:audioPlay(
      { after=wid, sync=true })

    cid = cmd:resume({ after=wid }, coid)
  end
end

print("cow say")
cmd:start({}, animation)
