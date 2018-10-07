local exdir = '../src/examples/search/'
package.path = package.path .. ";"..exdir.."?.lua"
--local spaces =
require('spaces')
spaces.storage('sw')
--spaces.debug()
local Levenshtein = require('levenshtein')
local SmallWorld = require('smallworld')
local Cosim = require('cosinesimilarity')

local storage = spaces.open("gloves")
local s = storage:open()
-----------------------------------------------------------------------------------------------
-- specify a glove text file which can be downloaded from here
-- http://nlp.stanford.edu/data/glove.6B.zip
-----------------------------------------------------------------------------------------------
local gdn = "../../glove/glove.6B.50d.txt"
local cnt = 1


spaces.setMaxMb(8000)
-----------------------------------------------------------------------------------------------
-- create a navigable small world index with faster parameters for building
-----------------------------------------------------------------------------------------------
local function createNSWBuild(container,dist)
    return SmallWorld(container,7,3,dist)
end
-----------------------------------------------------------------------------------------------
-- create a navigable small world index with more acurate parameters querying
-----------------------------------------------------------------------------------------------
local function createNSWSearch(container,dist)
    return SmallWorld(container,10,4,dist)
end
-----------------------------------------------------------------------------------------------
-- print a result
-----------------------------------------------------------------------------------------------
local function printResult(result,max,f)
    local r = 1
    for k,v in pairs(result) do

        if f then
            f(r,k,v)
        end
        r = r + 1
        if r > max then
            break
        end

    end
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
-----------------------------------------------------------------------------------------------
-- cosine similarity distance function considering vector
-----------------------------------------------------------------------------------------------
local function IndexedCosim(a,b)
    return 1-Cosim(a.v,b.v)
end
-----------------------------------------------------------------------------------------------
-- intialize and load
-----------------------------------------------------------------------------------------------
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
    swl = createNSWBuild(sl,Levenshtein)
    swg = createNSWBuild(sg,IndexedCosim)
    local cnt = 1
    local t = os.clock()
    for line in io.lines(gdn) do
        local word,v = split(line, " ")

        --print(cnt..".",word)

        words[word] = {v=v,w=word}
        swg:add(words[word])
        swl:add(word)

        cnt = cnt + 1
        if cnt % 100 == 0 then
            local result = swl:search(word)
            printResult(result,4)
        end

    end

    storage:commit()
else

    swl = createNSWSearch(s.leventy,Levenshtein)
    swg = createNSWSearch(s.glove,IndexedCosim)

end
local words = s.words

local searches = {"jear","yeer","japan","made", "coost","coast","noan","noun","moanin","moaning",",","taipan","taiwan","china","south","woman","female","laughter"}
for i,query in ipairs(searches) do
    print("search",query)
    local result = swl:search(query)
    local function semantic(i,k,v)
        --print("semantic","",i,k,v.value.w)
    end
    local function searched(i,k,v)
        --print("spelling",i,v.value)
        if i == 1 then
            printResult(swg:search(words[v.value]),5,semantic)
        end
    end
    printResult(result,4,searched)
    local t = os.clock()
    printResult(result,4,searched)
    print("hot",os.clock()-t)
end
