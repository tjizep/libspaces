local exdir = '../src/examples/search/'
package.path = package.path .. ";"..exdir.."?.lua"

local spaces = require('spaces')
spaces.storage('sw')
--spaces.debug()
local Levenshtein = require('levenshtein')
local SmallWorld = require('smallworld')
local Cosim = require('cosinesimilarity')

local storage = spaces.open("gloves")
local s = storage:open()
local WORLD_SIZE = 7
local SAMPLE_SIZE = 5
local gdn = "../../glove/glove.6B.50d.txt"
local cnt = 1


spaces.setMaxMb(3000)
-----------------------------------------------------------------------------------------------
-- create a navigable small world index
-----------------------------------------------------------------------------------------------
local function createNSW(container,dist)
    return SmallWorld(container,WORLD_SIZE,SAMPLE_SIZE,dist)
end
-----------------------------------------------------------------------------------------------
-- splits glove lines tokenized by sep into 50d vectors
-----------------------------------------------------------------------------------------------
local function split(inputstr, sep)
    if sep == nil then
        sep = "%s"
    end
    local w,t,i="",{},0
    for str in string.gmatch(inputstr, "([^"..sep.."]+)") do
        if i == 0 then
            w = str
        else
            t[i] = tonumber(str)
        end
        i = i + 1
    end
    return w,t
end
local swl
local swg
if s.leventy == nil then
    s.leventy = {} -- container for the levenshtein small world index
    s.glove = {}
    s.words = {}
    local sl = s.leventy
    local sg = s.glove
    local words = s.words
    -----------------------------------------------------------------------------------------------
    -- create smallworld index called 'leventy' with world size x and sample size y
    -- use larger values with bigger datasets or where the dimensionality or entropy of
    -- the values measured is high like thought vectors, documents, dna samples etc
    -----------------------------------------------------------------------------------------------
    swl = createNSW(sl,Levenshtein)
    swg = createNSW(sg,Cosim)
    local cnt = 1
    for line in io.lines(gdn) do
        local word,v = split(line, " ")

        print(cnt..".",word)
        words[cnt] = {word,v}
        --
        swg:add(v)
        swl:add(word)


        cnt = cnt + 1
        if cnt > 1000 then
           -- break
        end

        if cnt % 100 == 0 then
            print("lines",cnt)
        end

    end

    cnt = 1
    for line in io.lines(gdn) do
        local word,_ = split(line, " ")
        swl:add(word)
        cnt = cnt + 1
        if cnt > 1000 then
           -- break
        end

        if cnt % 100 == 0 then
            print("lines",cnt)
        end

    end
    storage:commit()
else
    swl = createNSW(s.leventy,Levenshtein)
    swg = createNSW(s.glove,Cosim)
end

local searches = {"jear","japan","made", "coost","noan","moanin",","}
for i,query in ipairs(searches) do
    print("search",query)
    local result = swl:search(query)
    local r = 0
    for k,v in pairs(result) do
        r = r + 1
        print(r,k,v.value)
        if r > 4 then
            break
        end

    end
end
