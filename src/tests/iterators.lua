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
local numbers = s:numbers()
print("all")

while numbers:valid() do
    print("iterate", numbers:key(), numbers:value())
    numbers:next()
end
numbers:start()
while numbers:valid() do
    print("after first", numbers:key(), numbers:value())
    numbers:next()
end
print("first")
numbers:start()
if numbers:first() then
    print("is first")

end
print(numbers:key(), numbers:value())
print("last")
--iter:last()
print(numbers:key(), numbers:value())
--
while numbers:valid() do
    print("after first", numbers:key(), numbers:value())
    numbers:next()
end
numbers:start()
local erased = 0
local count = #s.numbers
while numbers:valid() do
    print("count after erase",#s.numbers,"key before erase",numbers:key())
    s.numbers[numbers:key()] = nil
    print("key after erase",numbers:key(),numbers:value(),s.numbers[numbers:key()],"count after erase",#s.numbers)
    erased = erased+1
end
print("total erased",erased,count)
assert(count == erased, "erase failed")
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

