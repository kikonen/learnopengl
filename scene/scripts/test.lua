local rnd = math.random

function create_new_node(type)
  local opt = {
    type = type,
    pos = vec3(rnd(50), 1 + rnd(50), rnd(50)),
    rot = vec3(0, rnd(360), 0),
    scale = vec3(0.25 + rnd(1)),
  }
  local node_id = scene:create_node(opt)
  printf("created_node: %d\n", node_id)
end

print("loaded")

for i=0,10 do
  create_new_node("mirror_ball")
  create_new_node("planet_2")
  create_new_node("ball")
  create_new_node("ball_2")
  create_new_node("lamp_halo")
  create_new_node("brick_wall_1")
  create_new_node("skeleton_army")
end
