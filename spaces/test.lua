require "spaces"
local u = 1e6
local s = spaces.open();
if s.data == nil then
	s.data = "beans"
end
local k = s.data

--k = s.leaf
if k == nil then
	--print("did not find key, good!")
end
local t = os.clock()
k = s.data
print("current object count",#k)

if #k == 0 then
	k.l1 = 1
	k.l2 = 2
	k.l3 = 3
	k.l4 = 4
	print("current object count",#k)
	k[1] = 1
	k[2] = 2
	k[3] = 3
	k[4] = 4
	print("current object count",#k)
	for k,v in pairs(k) do
		print("(k,v) ",k,v)
	end
	print("start st write",t)

	local lt = {}
	
	local tl = 0
	spaces.begin()	
	for i = 1,u do
		local ss = ""..i
		tl = tl + #ss -- checksum
		k[ss] = i*2;
	end	
	print("end st write",os.clock()-t,tl,#k)

end

t = os.clock()
print("start st read",t)
local t2 = 0
for i = 1,u do
	local ss = ""..i
	t2 = t2 + #ss -- checksum
	if  k[ss] == nil then
		print("key error")
	end
end
if not t1 == t2 then
	print("checksum does not match")
end
print("end st read",os.clock()-t,t2)
t = os.clock()
print("start st iterate",t)
local cnt = 0
for k,v in pairs(k) do
	cnt = cnt + v
end
print("end st iterate",os.clock()-t,cnt)
--s.data = nil --delete everything added
spaces.commit()