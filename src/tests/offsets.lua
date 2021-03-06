--
-- tests iterator offsets

local spaces = require "spaces"
spaces.storage("test") -- puts data in the test subdirectory
local session = spaces.open("offsets") -- use offsets storage
local s = session:open() -- starts a transaction automatically
local U = 1e5
if s.offsets == nil  then
    local t = os.clock()
    local dl = 8
    local kl = 8
    s.offsets = {}
    local offsets = s.offsets
    for i = 1,U do
        offsets[i] = i
    end
    --session:commit()
    local dt = os.clock()-t;
    local ops = math.floor(U/dt)
    print("end st random write",dt,ops.." keys/s","key l.:"..kl,"data l.:"..dl)
else

end
local iter = s:offsets()
local base = U/2;
iter:move(base)
assert(iter:count()==U,"invalid move")
print(iter:count(),base,U)
for i = base,U do
    local r = math.floor(math.random(1, U-base))
   -- print("key at ",r)

    local k = iter:key(r)
    local test = k==(r+base)
    --print(U,k,r,s.offsets[r],test)

    assert(test,"invalid offset")
end
