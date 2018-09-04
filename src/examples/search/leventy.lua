local exdir = '../src/examples/search/'
package.path = package.path .. ";"..exdir.."?.lua"

local spaces = require('spaces')
local levenshtein = require('levenshtein')
local SmallWorld = require('smallworld')

spaces.storage("data")
-----------------------------------------------------------------------------------------------
-- create a smallworld index called 'leventy' with world size 2 and sample size 3
-- use larger values with bigger datasets or where the dimensionality or entropy of
-- the values measured is high like thought vectors, documents, dna samples etc
-----------------------------------------------------------------------------------------------
local storage = spaces.open("leventy")

local s = storage:open()

local sw = SmallWorld(s,2,3,levenshtein)
local words = {"mouse","able","baker","charlie","the","fire","truck","could","not" }
for w,word in ipairs(words) do
    sw:add(word)
end
local searches = {"bread","muose", "bakeri", "charles", "njet","yet","frack","fear"}
for i,query in ipairs(searches) do
    print("search",query)
    local result = sw:search(query)
    for k,v in pairs(result) do
        print(k,v.value)
    end
end

--return {create = Create}