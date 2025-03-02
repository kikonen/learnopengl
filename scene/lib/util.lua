print("LUA: MODULE util")

function printf(...)
  io.write(string.format(...))
end

function table_size(t)
  count = 0
  for _ in pairs(t) do count = count + 1 end
  return count
end

function kpairs(t)
  return next, t, nil
end

function pack_table(...)
  return { n = select("#", ...), ... }
end

function print_table(t, levels)
  printf("%s\n", format_table(t, "", levels or 5))
end

function format_table(t, indent, levels)
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
          v = format_table(v, indent .. "  ", levels - 1)
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

function dbg(levels, ...)
  local args = pack_table(...)
  for i = 1, args.n do
    printf("------------------\n")
    print_table(args[i], levels or 3)
  end
end

function trace(label, levels, ...)
  local args = pack_table(...)
  printf("[START: %s]\n", label)
  for i = 1, args.n do
    printf("----------%i/%i (%s)--------------------\n", i, args.n, label)
    print_table(args[i], levels or 3)
  end
  printf("------------------------------\n")
  printf("%s\n", debug.traceback())
  printf("[END: %s]\n", label)
end

function to_dream_mat4(src)
  local dst = dream.mat4.getIdentity()

  for i, row in ipairs(src) do
    for j, v in ipairs(row) do
      dst[(i - 1) * 4 + j] = v
    end
  end

  return dst
end
