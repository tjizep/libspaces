require "spaces"
--_G.type = require('mtype.pure').type
local _ = require ("moses")
local inspect = require('inspect')

local s = spaces.open();
local t = 	{ 	name="test", 
				age=5.1234,
				family=
				{	brother={name="brother1",age=13.1},
					mother={name="mother1",age=42.1},
					sister={},
					father={name="father1",age=44.1}

				}
			}
local t0 = { value="h4", position=1 }
local a = {}
a[s] = 1

s.data = t0
a[s.data] = 2
print("s.data",s.data)
print("a[s]",a[s],a[s.data])
for k,v in pairs(s) do	
	print(k,v)
end
for k,v in pairs(s.data) do
	print(k,v)
end
--print(inspect(s.data))
--print(s.data)
--print(s.data.value)
--print(s.data.position)
--print(t)

local t1 = {}
--t1[s.data] = 1
--t1[s] = 2

--print(t1[s.data],t1[s],s.data.value,s.data.position)
s.data = t
print(inspect(s))
s.data = nil
print(inspect(s))