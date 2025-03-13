require "scene/lib/util"
require "scene/lib/table_util"

local function bah()
end

a = {
  b="foo",
  c="bar",
  d={
    david=10,
    echo="2s",
    ditto=bah,
    e={
      zion1="bar1",
      zion2="bar2",
      zion3="bar3",
      zion4={},
      zion5=nil
    }
  }
}

print("========")
table_print(a)
print("========")
table_print(2)
print("========")
table_print(bah)
print("========")
table_print(nil)
print("========")
