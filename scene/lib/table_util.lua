print("LUA: MODULE table")

function kpairs(t)
  return next, t, nil
end

local function table_keys(t)
  local keys = {}
  for key,_ in pairs(t) do
    table.insert(keys, key)
  end
  return keys
end

function table_size(t)
  count = 0
  for _ in pairs(t) do count = count + 1 end
  return count
end

function table_pack(...)
  return { n = select("#", ...), ... }
end

function table_print(t, levels)
  printf("%s\n", table_format(t, "", levels or 5))
end

function table_format(t, indent, levels)
  if t == nil then
    return nil
  end

  local sb = ""

  indent = indent or ""
  levels = levels or 5

  if type(t) == "table" then
    if table_size(t) > 0 then
      local first = true
      for k, v in pairs(t) do
        if not first then
          sb = sb .. "\n"
        end

        sb = sb .. string.format("%s%s: ", indent, k)

        if type(v) == "table" and levels > 1 then
          v = table_format(v, indent .. "  ", levels - 1)
          if #v > 0 then
            sb = sb .. "\n"
          else
            v = "{}"
          end
        end
        sb = sb .. string.format("%s", v)

        first = false
      end
    end
  else
    sb = sb .. string.format("%s%s", indent, t)
  end

  return sb
end
