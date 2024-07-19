--printf("START: name=%s, id=%d, clone=%d", node:getName(), id, node:getCloneIndex())

local function attack(wid)
  --print(string.format("START: %d", id))
  return cid;
end

local function handler(coid)
  local wid = 0
  local cid = 0

  local origPos = node:getPos()

  while true do
    wid = cmd:wait({ after=cid, time=1 })

    cid = attack(wid)

    wid = cmd:wait({ after=cid, time=10 })

    cid = cmd:resume({ after=wid }, coid)
  end
end

--cmd:start({}, handler)

cmd:animationPlay(
  { after=wid, name = "Unreal Take" } )
