require "spaces"
spaces.storage("client")
spaces.observe("127.0.0.1",15003)
spaces.replicate("127.0.0.1",16003)
local storage = spaces.open(); -- starts a transaction automatically
local s = storage:open()

s.clientData = {name="",test={value="val",date={month=1,day=2,year=2018}} }
sorage:commit()
