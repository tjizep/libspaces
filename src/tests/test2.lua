require "libspaces"
local s = spaces.open();

s[1] = 0

--[[
s.data = {}
local k = s.data
print("current object count",#k)
k[2] = 10
print("current object count",#k)
]]