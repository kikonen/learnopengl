local Updater = {
  updaters = {}
}

function Updater:add_updater(fn)
  print("Updater:add_updater")
  print(fn)
  self.updaters[#self.updaters + 1] = fn
end

function Updater:update(dt)
  for i, fn in ipairs(self.updaters) do
    fn(dt)
  end
end

return Updater;
