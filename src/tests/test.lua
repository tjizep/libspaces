package.path = '~/torch/lua/?;~/torch/lua/?.lua;./?;./?.lua;../src/tests/?;../src/tests/?.lua;~/torch/lua/?/init.lua;;'
package.cpath = '~/torch/bin/?.so;~/torch/bin/lib?.so;./lib?.so;;'
require "spaces"
local u = 1e8
local kl = 8
local seed = 78976
local charset = {}  do -- [0-9a-zA-Z]
	for c = 48, 57  do table.insert(charset, string.char(c)) end
	for c = 65, 90  do table.insert(charset, string.char(c)) end
	for c = 97, 122 do table.insert(charset, string.char(c)) end
end
math.randomseed(seed)
local function randomString(length)
	if not length or length <= 0 then return '' end
	return randomString(length - 1) .. charset[math.random(1, #charset)]
end
local function randomNumber()

	return math.random(1, 1e4)*1e4+math.random(1, 1e4)
end
local function generate()
	local t = os.clock()
	local tdata = {}
	print("start st generating",t)
	local ls = 0
	for ri = 1,u do
		local s = randomString(kl);
		ls = ls + #s
		tdata[ri] = s
	end
	print("complete st generating",os.clock() - t, " avg. key len "..math.floor(ls/u))
	return tdata
end
spaces.begin()
local s = spaces.open(); -- starts a transaction automatically

if s == nil then
	s = {} -- nb. initialize the root space if its not initialized
end
local data = s.data

if s.data == nil then
	s.data = {}
end
data = s.data
print("current object count",#data)

local tdata = nil --generate()

if #data == 0 then


	spaces.begin()

	t = os.clock()
	local td = os.clock()
	print("start st write",t)
	for i = 1,u do
		local ss = nil

		if tdata == nil then
			ss = randomString(kl)
			if(i % (u/20)) == 0 then
				print("wrote "..(u/20) .. " keys in ",os.clock()-td .. " tot. "..i,(u/20)/(os.clock()-td).." keys/s")
				td=os.clock()
			end

		else
			ss = tdata[i]
		end

		data[ss] = i*2;
	end
	local dt = os.clock()-t;
	local ops = math.floor(u/dt)
	print("end st random write",dt,ops.." keys/s")

end

math.randomseed(seed)
print("start st read")
local cnt = 0
t = os.clock()
td = t
if tdata ~= nil then
	for _,v in ipairs(tdata) do
		cnt = cnt + 1
		local dv = data[v]

		if  dv == nil then
			print("key error")
		end

	end
	local dt = os.clock()-t;
	local ops = math.floor(cnt/dt)
	print("end st random read",dt,ops.." keys/s")
end

cnt = 0
t = os.clock()
local last = ""
for k,v in pairs(data) do
	cnt = cnt + 1
	if k < last then
		print("order error")
	end
	last = k
end
dt = os.clock()-t
print("end st iterate",dt,math.floor(cnt/dt).." keys/s")
--s.data = nil --delete everything added
spaces.commit()
--[[]]