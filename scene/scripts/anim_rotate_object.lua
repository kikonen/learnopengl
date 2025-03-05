--printf("START: name=%s, id=%d, clone=%d\n", node:get_name(), id, node:get_clone_index())

local function animation()
  local orig_pos = node:get_pos()
  local wid = 0
  local cid = 0
  local dir = 1

  local function animation_listener()
    wid = cmd:wait({ after=cid, time=0.5 })

    cid = cmd:rotate(
      { after=wid, time=3, relative=true },
      { 0, 1, 0 },
      dir * 45)

    cid = cmd:call(
      { after=cid },
      animation_listener)

    dir = -dir
  end

  wid = cmd:wait({ after=cid, time=1 })

  cid = cmd:call(
    { after=wid },
    animation_listener)
end

animation()
