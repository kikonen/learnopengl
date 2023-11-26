local luaNode = nodes[id]

local function animation(coid)
  print("animation:start", coid)
  local wid = -1
  local cid = -1
  local dir = 1

  wid = cmd:wait(cid, 1)

  while true do
    print("animation_loop:start")
    -- NOTE KI *WAIT* for resume to complete
    wid = cmd:wait(wid, 0.5)

    cid = cmd:rotate(
      0,
      { after=wid, time=10, relative=true },
      { 0, dir * 360, 0 })

    print("resume-animation...")
    cid = cmd:resume(coid, { after=cid }, "_")
    print("resumed-animation", cid)

    wid = cid
    dir = -dir

    -- NOTE KI wait for callback
    --coroutine.yield()
    print("animation_loop:end")
  end
  print("animation:end")
end

local function animation_2(coid)
  print("animation_2:start", coid)
  local wid = -1
  local cid = -1
  local dir = 1

  while true do
    print("animation_2_loop:start")
    wid = cmd:wait(cid, 1)

    local scale = { 1, 1, 1 }
    if dir < 0 then
      scale = { 4, 4, 4 }
    end

    cid = cmd:scale(
      0,
      { after=wid, time=20, relative=false },
      scale)
    print("scaled")

    print("resume-animation...")
    cid = cmd:resume(coid, { after=cid }, "callback")
    print("resumed-animation_2", cid)

    wid = cid
    dir = -dir

    print("animation_2_loop:end")
  end
  print("animation_2:end")
end

luaNode.start = function()
  local cid = -1
  --print(string.format("START: name=%s, id=%d, clone=%d", node:getName(), id, node:getCloneIndex()))

  local pos = node:getPos()

  print("node", node, id, pos)
  print("start_animation_2")

  print("cmd", cmd)

  cid = cmd:start(0, {}, animation_2)
  print("started", cid)

  print("start_animation")
  cid = cmd:start(0, {}, animation)
  print("started", cid)
end
