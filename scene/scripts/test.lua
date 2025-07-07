local rnd = math.random

function create_new_skeleton()
  create_new_node("skeleton_army")
end

function trigger_create_new_skeleton()
  need_create_node = true
end

function create_new_fps_counter()
  local opt = {
    type = util.sid("fps_counter"),
    id = "fps_counter",
    pos = vec3(2, -2, -1)
  }
  local node_id = scene:create_node(opt)
  printf("created_fps_coutner: %d\n", node_id)
end

function create_new_node(type)
  local opt = {
    type = util.sid(type),
    pos = vec3(rnd(50), 1 + rnd(50), rnd(50)),
    rot = vec3(0, rnd(360), 0),
    scale = vec3(0.25 + rnd(1)),
  }
  local node_id = scene:create_node(opt)
  printf("created_node: %d\n", node_id)
end

print("loaded")

function emit_particles()
  for _, state in pairs(states) do
    if state.emit_particles then
      state:emit_particles()
    end
  end
end

function sample()
  for i=0,10 do
    create_new_node("mirror_ball")
    create_new_node("planet_2")
    create_new_node("ball")
    create_new_node("ball_2")
    create_new_node("lamp_halo")
    create_new_node("brick_wall_1")
    create_new_node("skeleton_army")
  end

  emit_particles()
end

function cylon_armada()
  for i=0,10 do
    local opt = {
      type = util.sid("cylon_group"),
      pos = vec3(rnd(1000), 100 + rnd(500), 200 + rnd(500)),
      rot = vec3(0, rnd(360), 0),
      scale = vec3(0.01 + rnd(0.01)),
    }
    local node_id = scene:create_node(opt)
    printf("created_node: %d\n", node_id)
  end
end

function delete_test()
  local sid = util.sid("<flag>-9")
  scene:delete_node(sid)
end

-- sample()
-- cylon_armada()
create_new_node("water_ball")
create_new_node("pool")
create_new_node("cylon_group")

-- create_new_fps_counter()
-- create_new_node("flag_stand")

-- delete_test()

  emit_particles()
