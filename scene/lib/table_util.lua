print("LUA: MODULE table")

function kpairs(t)
  return next, t, nil
end

function table_keys(t)
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

function table_format(t, indent, levels, collected)
  if t == nil then
    return "<nil>"
  end

  if type(t) ~= "table" then
    return t
  end

  -- if table_size(t) == 0 then
  --   return "EMPTY"
  -- end

  local sb = {}

  collected = collected or {}
  indent = indent or ""
  levels = levels or 4

  collected[t] = true

  for k, v in pairs(t) do
    if type(v) == "table" and levels > 1 then
      if collected[v] then
        v = "<LOOP>"
      else
        v = table_format(v, indent .. "  ", levels - 1, collected)
        if #v == 0 then
          v = "{}"
        else
          v = "{\n  " .. indent .. v .. "}"
        end
      end
    else
      if type(v) == "string" then
        v = "\"" .. v .. "\""
      else
        v = string.format("%s", v)
      end
    end

    sb[#sb + 1] = k .. " = " .. v
  end

  local meta = getmetatable(t)
  if meta then
    local k = "[meta]"
    local v = table_format(meta, indent .. "  ", levels - 1, collected)
    v = "{\n  " .. indent .. v .. "}"

    sb[#sb + 1] = k .. " = " .. v
  end

  return table.concat(sb, "\n" .. indent)
end

function table_print(t, levels)
  printf("%s\n", table_format(t, "", levels or 5))
end
