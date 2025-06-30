print("LUA: MODULE util")

function printf(...)
  io.write(string.format(...))
end

function dbg(levels, ...)
  local args = table_pack(...)
  for i = 1, args.n do
    printf("------------------\n")
    table_print(args[i], levels or 3)
  end
end

function trace(label, levels, ...)
  local args = table_pack(...)
  printf("[START: %s]\n", label)
  for i = 1, args.n do
    printf("----------%i/%i (%s)--------------------\n", i, args.n, label)
    table_print(args[i], levels or 3)
  end
  printf("------------------------------\n")
  printf("%s\n", debug.traceback())
  printf("[END: %s]\n", label)
end

function run(script)
  local path = 'scene/scripts/' .. script .. '.lua'
  printf("run: %s\n", path)
  local fn = loadfile(path)
  if fn then
    fn()
  else
    printf("FAILED: %s\n", path)
  end
end
