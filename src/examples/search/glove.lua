local exdir = '../src/examples/search/'
package.path = package.path .. ";"..exdir.."?.lua"

local spaces = require('spaces')
local levenshtein = require('levenshtein')
local SmallWorld = require('smallworld')

local storage = spaces.open("gloves")
local s = storage:open()

local gdn = "../../glove/glove.6B.50d.txt"
local cnt = 1

-----------------------------------------------------------------------------------------------
-- splits lines tokenized by sep into 1d vectors
-----------------------------------------------------------------------------------------------
function split(inputstr, sep)
    if sep == nil then
        sep = "%s"
    end
    local t={} ; i=1
    for str in string.gmatch(inputstr, "([^"..sep.."]+)") do
        t[i] = str
        i = i + 1
    end
    return t
end

if s.leventy == nil then
    s.leventy = {} -- container for the levenshtein small world index
    local sl = s.leventy
    -----------------------------------------------------------------------------------------------
    -- create smallworld index called 'leventy' with world size 7 and sample size 5
    -- use larger values with bigger datasets or where the dimensionality or entropy of
    -- the values measured is high like thought vectors, documents, dna samples etc
    -----------------------------------------------------------------------------------------------
    local swl = SmallWorld(sl,7,5,levenshtein)


    for line in io.lines(gdn) do
        local parts = split(line, " ")
        local word = parts[1]
        print(word)
        swl:add(word)

        cnt = cnt + 1
        if cnt % 10000 == 0 then
            print("lines",cnt)
        end

    end

end
