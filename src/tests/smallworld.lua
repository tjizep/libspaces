----------------------------------------------------------------------------------------------------
-- a persisted metricized small world index in lua
-- lua/spaces/persisted version by Christiaan Pretorius chrisep2@gmail.com
-- based on java code by Alexander Ponomarenko aponom84@gmail.com
-- NB: requires caller/user to seed lua internal pseudo random generator for sufficient randomness
----------------------------------------------------------------------------------------------------

local spaces = require "spaces"
----------------------------------------------------------------------------------------------------
-- creates a new sw object for use
--
function Create(spaces,name,calcDistance)
    -- spaces.storage(storage) should be called before this function
    ------------------------------------------------------------------------------------------------
    ---
    --- closure variables initialization
    ---
    ------------------------------------------------------------------------------------------------
    local session = spaces.open(name) -- use offsets storage
    local temp = spaces.open("temp"..name..os.clock())
    local perm = session:open() -- starts a transaction automatically
    local s = perm:open()
    local ts = temp:open()
    local nodeCount = 0

    ------------------------------------------------------------------------------------------------
    -- initialize the persistent data structures
    function Nodes()
        if s.index==nil then
            s.index = {nodes={}}
        end
        return s.index.nodes
    end

    ------------------------------------------------------------------------------------------------
    -- initialize global view sets
    function GlobalUnorderedViewSet()
        ts.globalUnordered = {}
        return ts.globalUnordered
    end
    function GlobalViewed()
        ts.globalViewed = {}
        return ts.globalViewed
    end

    ------------------------------------------------------------------------------------------------
    -- create temporary canditates
    function Candidates()

        ts.candidates = {}

        return ts.candidates
    end

    ------------------------------------------------------------------------------------------------
    -- create temporary viewed set
    function Viewed()

        ts.viewed = {}

        return ts.viewed
    end

    ------------------------------------------------------------------------------------------------
    -- create temporary viewed set
    function VisitedUnordered()
        ts.viewedUnordered = {}
        return ts.viewedUnordered
    end

    ------------------------------------------------------------------------------------------------
    -- create temporary viewed setviewed
    function Visited()
        ts.visited = {}
        return ts.visited
    end

    ------------------------------------------------------------------------------------------------
    -- initialize search
    function InitializeSearch()
        return Nodes(), GlobalUnorderedViewSet(),GlobalViewed()
    end

    ------------------------------------------------------------------------------------------------
    -- create a node template in the approximate voromoi graph for connection to others
    function NodeTemplate()
        return { friends={} }
    end

    ------------------------------------------------------------------------------------------------
    -- return a random node
    function getRandomNode(nodes)
        if nodeCount == 0 then
            nodeCount = #nodes
        end
        local randomKey = nodes():key(math.random(1,nodeCount))         -- this will take long if the set is large
        node = nodes[randomKey]
        return {name=randomKey,value=node.value,friends=node.friends}   -- copy values
    end

    ------------------------------------------------------------------------------------------------
    -- return the k.th or last distance
    function kDistance(set,k)
        return set():key(math.mininteger(k,#set))
    end

    ------------------------------------------------------------------------------------------------
    --- Per Instance functions, they have a self in front
    ------------------------------------------------------------------------------------------------
    -- search nodes
    -- the similarity with dijkstras algorithm is not coincidental or accidental
    local function searchNodes(self,query,k,entry, globalUnordered)
        local lowerBound = 0
        local candidates = Candidates()
        local viewed = Viewed()
        local visitedUnordered = VisitedUnordered()
        entry.distance = calcDistance(query.value,entry.value)
        globalUnordered[entry.name] = entry
        visitedUnordered[entry.name] = entry
        candidates[entry.distance] = entry --- the random starting point
        viewed[entry.distance] = entry
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
                    local distance = calcDistance()
                    local candidate = { name=node.name, value=node.value, friends=node.friends }
                    candidate.distance = calcDistance(query.value,candidate.value)
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
    -- search a node
    local function search (self,query,k,sampleSize)

        local nodes,globalUnordered,global = InitializeSearch()

        for i=1,sampleSize do
            local viewed = searchNodes(self,query,k,getRandomNode(nodes),globalUnordered)
            for k,v in pairs(viewed) do
                global[k] = v
            end
        end
    end

    ------------------------------------------------------------------------------------------------
    -- add a node
    -- name should be unique
    function add(self,k,name,value)
        local nodes = Nodes()

        local query = { name=name,value=value,friends={} }
        if nodes():empty() then
            nodes[name] = query
            return
        end
        if not nodes[name] == nil then
            error("node "..name.." already exists ")
        end

        local viewed = search(self,query,k,7)

        nodes[name] = query
        local toadd = nodes[name] -- use the persisted version
        local i = 0
        for other,value in pairs(viewed) do
            if i >= k then
                break
            end
            i = i + 1
            -- two way friends
            toadd.friends[other] = value
            value.friends[name] = toadd
        end


    end

    ------------------------------------------------------------------------------------------------
    -- the exposed library object
    ------------------------------------------------------------------------------------------------
    local smallWorld =
    {   -- called as sw:search
    ,   search = search
        -- called as sw:add
    ,   add = add
    }

    return smallWorld
end
------------------------------------------------------------------------------------------------
-- a levenshtein distance on strings
local function levenshtein(a,b)
end
spaces.storage('test')
local sw = Create(spaces,"leventy",levenshtein)
local k = 5 -- max friend count - such a small world this is
sw:add(k,1,"hello")
sw:add(k,2,"hallo")
sw:add(k,3,"banana")
sw:add(k,4,"bandana")

--return {create = Create}