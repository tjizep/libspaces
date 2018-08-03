local spaces = require "spaces"
spaces.storage("ranges") -- puts data in the jsonp subdirectory
local session = spaces.open()
local s = session:open(); -- starts a transaction automatically
if s.alphan == nil then
    print("importing")
    s.alphan = {a=0,b=2,d=4,e=5,f=6,g=7,child={1,2,3,4,5} }
    s.numbers = {[0.01]=0,[2]=1,[4.1]=2,[5.15]=3,[6.0]=4,[7.08]=5,[4.05]={1,2,3,4,5} }
    s.array = {0,2,4,{1,2,3,4,5},6,8,10 }
    s.ordered = s.alphan
    session:commit()
else
    print("loading...")
end
local iter = s:numbers()
print("all")

while iter:valid() do
    print("iterate",iter:key(),iter:value())
    iter:next()
end
iter:start()
while iter:valid() do
    print("after first",iter:key(),iter:value())
    iter:next()
end
print("first")
iter:start()
if iter:first() then
    print("is first")

end
print(iter:key(),iter:value())
print("last")
--iter:last()
print(iter:key(),iter:value())
--


print("tn")
local tn = s:numbers(2.0001,6.01)
local t = s:alphan("b","e")
local t1 = s:ordered("b","e")
local ta = s:array(2,6)
print("t")
for k,v in t do
    print("",k,v)
end
print("t1")
for k,v in t1 do
    print("",k,v)
end
print("tn")
for k,v in tn do
    print("",k,v)
end
print("ta")
for k,v in ta do
    print("",k,v)
end

