local spaces = require "paces"
spaces.storage("client")
spaces.observe("127.0.0.1",15003)
spaces.replicate("127.0.0.1",16003)
local s = spaces.open()
s.clientData = {name="",test={value="val",date={month=1,day=2,year=2018}} }
spaces.commit()
