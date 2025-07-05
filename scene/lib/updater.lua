local Updater = {
  updaters = {}
}

function Updater:update(dt)
  for _, fn in ipairs(self.updaters) do
    fn(dt)
  end
end

function Updater:refresh(dt)
  local updaters = {}
  for _, state in pairs(states) do
    if state.updaters then
      for _, fn in ipairs(state.updaters) do
        updaters[#updaters + 1] = fn
      end
    end
  end
  printf("UPDATER: count=%d\n", #updaters)

  self.updaters = updaters
end

return Updater;
