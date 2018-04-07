package.path = '~/torch/lua/?;~/torch/lua/?.lua;~/torch/lua/?/init.lua;;'
package.cpath = '~/torch/bin/?.so;./?.so;;'
require "libspaces"
local u = 1e6
local s = spaces.open();
local tdata = {}
local data = s.data
if not data == nil then
	print("ERROR: find non existent key")
end

local charset = {}  do -- [0-9a-zA-Z]
	for c = 48, 57  do table.insert(charset, string.char(c)) end
	for c = 65, 90  do table.insert(charset, string.char(c)) end
	for c = 97, 122 do table.insert(charset, string.char(c)) end
end
math.randomseed(78976)
local function randomString(length)
	if not length or length <= 0 then return '' end
	return randomString(length - 1) .. charset[math.random(1, #charset)]
end
local function randomNumber()

	return math.random(1, 1e4)*1e4+math.random(1, 1e4)
end

local t = os.clock()
print("start st generating",t)
for ri = 1,u do
	tdata[ri] = randomString(8)
end
print("complete st generating",os.clock() - t)

if s.data == nil then
	s.data = {}
end

data = s.data

print("current object count",#data)
if #data == 0 then


	local lt = {}
	
	local tl = 0
	spaces.begin()


	t = os.clock()
	local td = os.clock()
	print("start st write",t)
	for i = 1,u do
		local ss = tdata[i]
		tl = tl + i*2 -- checksum
		data[ss] = i*2;
		--[[
		-- if (i % (u/10)) == 0 then

			print("continue st write",i,os.clock()-td)
			td = os.clock()
		end
		]]
	end	
	print("end st write",os.clock()-t,tl)

end


print("start st read")
local cnt = 0
t = os.clock()
td = t
for _,v in ipairs(tdata) do
	cnt = cnt + 1
	local dv = data[v]
	if  dv == nil then
		print("key error")
	end

end
print("end st read",os.clock()-t,cnt)
cnt = 0
t = os.clock()
for k,v in pairs(data) do
	cnt = cnt + 1
end
print("end st iterate",os.clock()-t,cnt)
--s.data = nil --delete everything added
spaces.commit()
--[[]]