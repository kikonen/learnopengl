local Updater = {
  updaters = {}
}

function Updater:add_updater(fn)
  -- printf("Updater:add_updater: %s\n", fn)
  self.updaters[#self.updaters + 1] = fn
  -- table_print(self)
end

function Updater:remove_updater(fn)
  -- TODO KI HOW?!?
  --self.updaters[#self.updaters + 1] = fn
end

function Updater:update(dt)
  for i, fn in ipairs(self.updaters) do
    -- printf("update: %s\n", fn)
    fn(dt)
  end
end

return Updater;
