require "packages"
require "spaces"
--spaces.debug()
spaces.storage("test")
--spaces.observe("localhost",15003)
--spaces.replicate("localhost",16003)
--spaces.localWrites(false)
local u = 1e6
local MAX_GEN =3e6
local kl = 8
local seed = 78976
math.randomseed(seed) -- reseed to standard value for repeatable tests

local charset = {}  do -- [0-9a-zA-Z]
	for c = 48, 57  do table.insert(charset, string.char(c)) end
	for c = 65, 90  do table.insert(charset, string.char(c)) end
	for c = 97, 122 do table.insert(charset, string.char(c)) end
end

local function randomString(length)
	if not length or length <= 0 then return '' end
	return randomString(length - 1) .. charset[math.random(1, #charset)]
end

local function randomNumber()
	return math.random(1, 3e4)*math.random(1, 3e4)
end

local generator = randomString

local function generate(n)
	--local t = os.clock()
	local tdata = {}
	--print("start st generating",t)
	local ls = 0
	for ri = 1,n do
		local s = generator(kl)
		ls = ls + #s
		tdata[ri] = s
	end
	--print("complete st generating",os.clock() - t, " avg. key len "..math.floor(ls/u))
	return tdata
end
spaces.setMaxMb(5000)

local s = spaces.open(); -- starts a transaction automatically

if s == nil then
	print("intializing root")
	s = {} -- nb. initialize the root space if its not initialized
end
local data = s.data
print("data: ", data)
if data == nil then
	s.data = {}
	data = s.data
end
if data == nil then

end
print("data: ", data)
print("current object count",#data)

local tdata = {}

if #data == 0 or #data < u then

	local PERIOD = 1e6
	local t = os.clock()
	local td = os.clock()
	print("start st write",t)
	local ustart = 0
	for i = 1,u do
		local ss = tdata[i-ustart]
		if ss == nil then
			local gt = os.clock()
			tdata = generate(math.min(u,MAX_GEN))
			ustart = i - 1
			gt = (os.clock() - gt)
			td = td + gt
			t = t + gt
			ss = tdata[i-ustart]
		end
		if (i % PERIOD) == 0 then
			print("wrote "..(PERIOD) .. " keys in ",os.clock()-td .. " tot. "..i,(PERIOD)/(os.clock()-td).." keys/s")
			td=os.clock()
		end

		data[ss] = math.random(1, 3e4);
		--spaces.commit()

	end
	local dt = os.clock()-t;
	local ops = math.floor(u/dt)
	print("end st random write",dt,ops.." keys/s")
	spaces.commit()

end
math.randomseed(seed)
tdata = generate(math.min(u,MAX_GEN))
spaces.read()
for rr = 1,2 do
	print("start st read")
	local cnt = 0
	t = os.clock()
	td = t
	if tdata ~= nil then
		local lt = 0
		local mt = 0
		for k,v in ipairs(tdata) do
			cnt = cnt + 1
			local dv = data[v]
			if  dv == nil then
				mt = mt + 1
			else
				lt = lt +1
			end

		end
		local dt = os.clock()-t;
		local ops = math.floor(cnt/dt)
		print("end st random read",dt,ops.." keys/s",lt,mt)
	end
end
cnt = 0
t = os.clock()
local last = ""
for k,v in pairs(data) do
	cnt = cnt + 1
	if k < last then
		--error("order error")
	end
	last = k
end
dt = os.clock()-t
print("end st iterate",dt,math.floor(cnt/dt).." keys/s")
--s.data = nil --delete everything added

--[[]]