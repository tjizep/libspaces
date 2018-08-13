----------------------------------------------------------------------------------------------------
-- a persisted metricized small world index in lua
-- lua/spaces/persisted version by Christiaan Pretorius chrisep2@gmail.com
-- based on java code by Alexander Ponomarenko aponom84@gmail.com
-- NB: requires caller/user to seed lua internal pseudo random generator for sufficient randomness
----------------------------------------------------------------------------------------------------
--local inspect = require "inspect_meta"

local spaces = require "spaces"
----------------------------------------------------------------------------------------------------
-- creates a new sw object for use
--
local function Create(name,k,sampleSize,calcDistance)
    -- spaces.storage(storage) should be called before this function
    ------------------------------------------------------------------------------------------------
    ---
    --- closure variables initialization
    ---
    ------------------------------------------------------------------------------------------------

    local perm = spaces.open(name) -- use offsets storage
    local temp = spaces.open("temp"..name..os.clock())
    local s = perm:open()
    local ts = temp:open()

    ------------------------------------------------------------------------------------------------
    -- initialize the persistent data structures
    function Nodes()
        if s.index==nil then
            s.index = {nodes={}}
        end
        return s.index.nodes
    end
    function Stats()
        if s.stats==nil then
            s.stats = {count=0}
        end
        return s.stats
    end
    ------------------------------------------------------------------------------------------------
    -- initialize temporary global view sets
    local function GlobalUnorderedViewSet()
        --ts.globalUnordered = {}
        return {} --ts.globalUnordered
    end
    ------------------------------------------------------------------------------------------------
    -- ordered global view set
    local function GlobalViewed()
        ts.globalViewed = {}
        return ts.globalViewed
    end

    ------------------------------------------------------------------------------------------------
    -- create ordered temporary canditates
    local function Candidates()
        ts.candidates = {}
        return ts.candidates
    end

    ------------------------------------------------------------------------------------------------
    -- create ordered temporary viewed set
    local function Viewed()
        ts.viewed = {}
        return ts.viewed
    end

    ------------------------------------------------------------------------------------------------
    -- create temporary viewed set
    local function VisitedUnordered()
        --ts.visitedUnordered = {}
        return {} --ts.visitedUnordered
    end

    ------------------------------------------------------------------------------------------------
    -- create temporary viewed setviewed
    local function Visited()
        ts.visited = {}
        return ts.visited
    end

    ------------------------------------------------------------------------------------------------
    -- initialize search
    local function InitializeSearch()
        return Nodes(), GlobalUnorderedViewSet(),GlobalViewed()
    end

    ------------------------------------------------------------------------------------------------
    -- create a node template in the approximate voromoi graph for connection to others
    local function NodeTemplate()
        return { friends={} }
    end

    ------------------------------------------------------------------------------------------------
    -- return a random node
    local function getRandomNode(nodes)
        local randomKey = math.random(1,Stats().count)
        local node = nodes[randomKey]
        return {name=randomKey,value=node.value,friends=node.friends}   -- copy values
    end

    ------------------------------------------------------------------------------------------------
    -- return the k.th or last distance
    local function kDistance(set,k)
        return set():key(math.min(k,#set))
    end

    ------------------------------------------------------------------------------------------------
    --- Per Instance functions, they have a self in front
    ------------------------------------------------------------------------------------------------
    -- search nodes
    -- the similarity with dijkstras algorithm is neither coincidental nor accidental
    local function searchNodes(query,entry, globalUnordered)
        local lowerBound = 0
        local candidates = Candidates()
        local viewed = Viewed()
        local visitedUnordered = VisitedUnordered()
        local distance = calcDistance(query,entry.value)
        local candidate = { name=entry.name, value=entry.value, friends=entry.friends,distance=distance }
        globalUnordered[entry.name] = candidate
        visitedUnordered[entry.name] = candidate
        candidates[distance] = candidate --- the random starting point
        viewed[distance] = candidate
        while not candidates():empty() do
            local current = candidates():firstValue()
            candidates[candidates():firstKey()] = nil

            --- find the k lower bound of the current ordered viewed set
            lowerBound = kDistance(viewed, k);
            if current.distance > lowerBound then
                break
            end

            --- remember which have been visited for this instance of the k search
            visitedUnordered[current.name] = current
            --- look at the friends of the current candidate
            --- attempt to find a closer friend of friends
            for dist,node in pairs(current.friends) do
                --- do not visit the node again
                if globalUnordered[node.name] == nil then
                    local candidate = { name=node.name, value=node.value, friends=node.friends }
                    candidate.distance = calcDistance(query,candidate.value)
                    --- remember the closest nodes for all k instances of k search
                    globalUnordered[node.name] = node
                    --- add candidates in order of closeness
                    candidates[candidate.distance] = candidate
                    --- add viewed set in order of closeness
                    viewed[candidate.distance] = candidate
                end
            end
        end

        for k,v in pairs(visitedUnordered) do
            globalUnordered[k] = v
        end

        return viewed -- visitedUnordered
    end

    ------------------------------------------------------------------------------------------------
    -- finds nodes closest to query parameter according to distance function
    local function search (self,query)

        local nodes,globalUnordered,global = InitializeSearch()

        for i=1,sampleSize do
            local viewed = searchNodes(query,getRandomNode(nodes),globalUnordered)
            for k,v in pairs(viewed) do
                global[k] = v
            end
        end
        return global
    end

    ------------------------------------------------------------------------------------------------
    -- add a node
    -- name should be unique
    local function add(self,value)
        local nodes = Nodes()
        local stats = Stats()
        local name = stats.count + 1
        local query = { name=name,value=value,friends={} }
        if not nodes[name] == nil then
            error("node "..name.." already exists ")
        end

        if nodes():empty() then
            stats.count = stats.count + 1
            nodes[name] = query
            return
        end


        local viewed = search(self,value)
        nodes[name] = query
        stats.count = stats.count + 1
        local toadd = nodes[name] -- use the persisted version
        local i = 0
        for other,value in pairs(viewed) do
            if i >= k then
                break
            end
            i = i + 1
            -- two way friends
            local pvalue = nodes[value.name] -- reaquire from storage
            toadd.friends[pvalue.name] = pvalue
            pvalue.friends[name] = toadd
        end
    end

    ------------------------------------------------------------------------------------------------
    -- the exposed library object
    ------------------------------------------------------------------------------------------------
    local smallWorld =
    {   -- called as sw:search
        search = search
        -- called as sw:add
    ,   add = add
    }

    return smallWorld
end

------------------------------------------------------------------------------------------------
-- a levenshtein distance on strings
-- from https://github.com/kennyledet/Algorithm-Implementations/blob/master/...
-- Levenshtein_distance/Lua/Yonaba/levenshtein.lua
------------------------------------------------------------------------------------------------
local function min(a, b, c)
    return math.min(math.min(a, b), c)
end

-- Creates a 2D matrix
local function matrix(row,col)
    local m = {}
    for i = 1,row do m[i] = {}
        for j = 1,col do m[i][j] = 0 end
    end
    return m
end

-- Calculates the Levenshtein distance between two strings
local function levenshtein(a, b)
    local M = matrix(#a +1,#b +1)
    local i,j,cost
    local row,col = #M,#M[1]
    for i = 1,row do M[i][1] = i-1 end
    for j = 1,col do M[1][j] = j-1 end
    for i = 2,row do
        for j = 2,col do
            if (a:sub(i-1,i-1) == b:sub(j-1,j-1)) then cost = 0
            else cost = 1
            end
            M[i][j] = min(M[i-1][j]+1,M[i][j-1]+1,M[i-1][j-1]+cost)
        end
    end
    return M[row][col]
end
spaces.storage("test")

local sw = Create("leventy",5,7,levenshtein)
local words = {"mouse","able","baker","charlie","the","fire","truck","could","not" }
for w,word in ipairs(words) do
    sw:add(word)
end
local searches = {"bread","muose","bakeri", "charles", "njet","yet"}
for i,query in ipairs(searches) do
    print("search",query)
    local result = sw:search(query)
    for k,v in pairs(result) do
        print(k,v.value)
    end
end

--return {create = Create}