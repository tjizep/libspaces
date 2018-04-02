require "spaces"
local u = 1e6
local s = spaces.open();
if s.data == nil then
	s.data = "beans"
end
local k = s.data
if not k == nil then
	print("ERROR: find non existent key")
end
local t = os.clock()
s.data = {}
k = s.data
print("current object count",#k)
if #k == 0 then
	k[1] = 1
	k[2] = 2
	k[3] = 3
	k[4] = 4
	print("current object count",#k)

	k.l1 = 1
	k.l2 = 2
	k.l3 = 3
	k.l4 = 4
	print("current object count",#k)
	
	for k,v in pairs(k) do
		print("(k,v) ",k,v)
	end
	print("start st write",t)

	local lt = {}
	
	local tl = 0
	spaces.begin()	
	for i = 1,u do
		local ss = ""..i --
		tl = tl + i*2 -- checksum
		k[ss] = i*2;
	end	
	print("end st write",os.clock()-t,tl)

end

t = os.clock()
print("start st read",t)
local t2 = 0
for i = 1,u do
	local ss = ""..i --
	if  k[ss] == nil then
		print("key error")
	end
	--t2 = t2 + k[ss] -- checksum
end
if not t1 == t2 then
	print("checksum does not match")
end
print("end st read",os.clock()-t)
t = os.clock()
print("start st iterate",t)
local cnt = 0
local ops = 0;
local opst = 0;
for k,v in pairs(k) do
	local lv = v
	cnt = cnt + v
	opst = opst + (lv + ((lv *10)/ 10) ) - (lv - ((lv *10)/ 10) ) - lv
	ops = ops + (v + ((v *10)/ 10) ) - (v - ((v *10)/ 10) ) - v
end
if not (cnt == ops and opst == ops) then
	print("ERROR end st iterate",os.clock()-t,cnt, ops, opst)
else
	print("end st iterate",os.clock()-t,cnt, ops, opst)
end

--s.data = nil --delete everything added
spaces.commit()
--[[]]