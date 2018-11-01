-----------------------------------------------------------------------------------------------
-- search example to import and query glove vectors
-- usage example: lua search.lua ~/[Path to]/glove.6B.50d.txt
-- specify a glove text file which can be downloaded from here
-- http://nlp.stanford.edu/data/glove.6B.zip
-----------------------------------------------------------------------------------------------
local exdir = '../src/examples/search/'
package.path = package.path .. ";"..exdir.."?.lua"
if arg[1] == nil then
    print("usage example: lua search.lua ~/[Path to]/glove.6B.50d.txt")
    return -1
end
if _ENV then
    spaces = require('spaces')
else
    require('spaces')
end
spaces.storage('sw')

local Levenshtein = require('levenshtein')
local SmallWorld = require('smallworld')
local Cosim = require('cosinesimilarity')

local session = spaces.open("gloves")
local s = session:open()
-----------------------------------------------------------------------------------------------
-- specify a glove text file which can be downloaded from here
-- http://nlp.stanford.edu/data/glove.6B.zip
-----------------------------------------------------------------------------------------------
local gdn = arg[1] -- "../../glove/glove.6B.50d.txt"
local cnt = 1

--- set the max memory use for spaces
spaces.setMaxMb(1500)

-----------------------------------------------------------------------------------------------
-- create a navigable small world index with faster parameters for building
-----------------------------------------------------------------------------------------------
local function createNSWBuild(container,dist)
    return SmallWorld(container,7,3,dist)
end
-----------------------------------------------------------------------------------------------
-- create a navigable small world index with more accurate parameters for querying
-----------------------------------------------------------------------------------------------
local function createNSWSearch(container,dist)
    return SmallWorld(container,7,4,dist)
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
local words
-----------------------------------------------------------------------------------------------
-- ouput print functions to monitor progress
-----------------------------------------------------------------------------------------------
local function semantic(i,k,v)
    print("semantic","",i,k,v.value.w)
end
-----------------------------------------------------------------------------------------------
-- ouput print functions to monitor progress
-----------------------------------------------------------------------------------------------
local function searched(i,k,v)
    print("spelling",i,v.value)
    if i == 1 then
        printResult(swg.search(words[v.value]),6,semantic)
    end
end

if s.leventy == nil then
    s.leventy = {} -- container for the levenshtein small world index
    s.glove = {}
    s.words = {}

    local sl = s.leventy
    local sg = s.glove
    words = s.words

    -----------------------------------------------------------------------------------------------
    -- create a smallworld index called 'leventy' with world size x and sample size y
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
        swg.add(words[word])
        swl.add(word)
        --
        cnt = cnt + 1
        if cnt % 100 == 0 then
            print("lines added",cnt)

            --local result = swl.search(word)
            --printResult(result,4,searched)
        end

    end
    print("saving...")
    session:commit()
else

    swl = createNSWSearch(s.leventy,Levenshtein)
    swg = createNSWSearch(s.glove,IndexedCosim)

end
words = s.words

local searches = {"taiwan","china", "formosa","synonym","colloquially","jear","yeer","japan","made", "coost","coast","noan","noun","moanin","moaning",",","taipan","south","woman","female","laughter"}
for i,query in ipairs(searches) do
    print("search",query)

    local result = swl.search(query)
    printResult(result,4,searched)



end