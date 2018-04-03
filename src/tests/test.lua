require "libspaces"
local u = 1e8
local s = spaces.open();
if s.data == nil then
	s.data = "beans"
end
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


local t = os.clock()
local td = os.clock()
s.data = {}
data = s.data
print("current object count",#data)
if #data == 0 then
	print("start st write",t)

	local lt = {}
	
	local tl = 0
	spaces.begin()	
	for i = 1,u do
		local ss = randomString(8) --td = os.clock()
		tl = tl + i*2 -- checksum
		data[ss] = i*2;
		if (i % (u/50)) == 0 then
			print("continue st write",i,os.clock()-td)
			td = os.clock()
		end
	end	
	print("end st write",os.clock()-t,tl)

end

t = os.clock()
print("start st iterate",t)
local cnt = 0
local ops = 0;
local opst = 0;
for k,v in pairs(data) do

	local dv = data[k]
	if  dv == nil then
		print("key error")
	end

end
print("end st read/iterate",os.clock()-t)
--s.data = nil --delete everything added
spaces.commit()
--[[]]