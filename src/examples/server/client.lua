require "spaces"
spaces.storage("data")
spaces.observe("127.0.0.1",15003)
spaces.replicate("127.0.0.1",16003)
local session = spaces.open("client"); -- starts a transaction automatically
local s = session:open()

s.clientData = {name="",test={value="val",date={month=1,day=2,year=2018}} }
session:commit()
