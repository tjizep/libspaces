local spaces = require "spaces"
spaces.storage("transactions")
local u = 1e5
local session = spaces.open()
local s = session:open()

if s.transactions then
    print("found",s.transactions.value)
else
    s.transactions = {value=0}
end


local t = os.clock()
local transactions = s.transactions
for tx=1,u do
    transactions.value=transactions.value + 1
    session:commit()
end
print(s.transactions.value,math.floor(u/(os.clock()-t)).." tps")