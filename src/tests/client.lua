require "packages"
require "spaces"
spaces.data("client")
spaces.replicate("127.0.0.1")
local s = spaces.open()
s.clientData = {name="",test={value="val",date={month=1,day=2,year=2018}} }
spaces.commit()
