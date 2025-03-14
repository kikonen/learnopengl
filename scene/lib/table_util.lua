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

function table_print(t, levels)
  printf("%s\n", table_format(t, "", levels or 5))
end

function table_format(t, indent, levels)
  if t == nil then
    return "<nil>"
  end

  if type(t) ~= "table" then
    return t
  end

  if table_size(t) == 0 then
    return ""
  end

  local sb = {}

  indent = indent or ""
  levels = levels or 5

  for k, v in pairs(t) do
    if type(v) == "table" and levels > 1 then
      v = table_format(v, indent .. "  ", levels - 1)

      if #v == 0 then
        v = "{}"
      else
        v = "{\n  " .. indent .. v .. "}"
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

  return table.concat(sb, "\n" .. indent)
end
