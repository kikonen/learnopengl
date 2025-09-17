print("LOAD: class_actor")

local Actor = Node:new()

function Actor:start()
  printf("start: %d\n", self.id or 0)
end

function Actor:update(dt)
  printf("updated: %d, dt=%f\n", self.id or 0, dt or 0)
end

return Actor
