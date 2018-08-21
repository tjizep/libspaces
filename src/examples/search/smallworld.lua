----------------------------------------------------------------------------------------------------
-- a persisted metricized small world index in lua and spaces
-- lua/spaces/persisted version by Christiaan Pretorius chrisep2@gmail.com
-- based on java code by Alexander Ponomarenko aponom84@gmail.com
-- NB: requires caller/user to seed lua internal pseudo random generator for sufficient randomness
----------------------------------------------------------------------------------------------------
--local inspect = require "inspect_meta"

local spaces = require "spaces"
----------------------------------------------------------------------------------------------------
-- creates a new sw object for use
--
local function Create(groot,worldSize,sampleSize,calcDistance)
    -- spaces.storage(storage) should be called before this function
    ------------------------------------------------------------------------------------------------
    ---
    --- closure variables initialization
    ---
    ------------------------------------------------------------------------------------------------
    local ival =1

    local temp = spaces.open("temp"..os.clock())
    local ts = temp:open()
    ------------------------------------------------------------------------------------------------
    -- initialize the persistent data structures
    local function Nodes()
        if groot.index==nil then
            groot.index = {nodes={}}
        end
        return groot.index.nodes
    end
    local function Stats()
        if groot.stats==nil then
            groot.stats = {count=0}
        end
        return groot.stats
    end
    ------------------------------------------------------------------------------------------------
    -- alloc temp instance
    local function instance(val)
        local c = ival
        ival = ival + 1
        val[c] = {}
        return val[c]
    end
    ------------------------------------------------------------------------------------------------
    -- initialize temporary global view sets
    local function GlobalUnorderedViewSet()
        if ts.globalUnordered == nil then
            ts.globalUnordered = {}
        end
        return instance(ts.globalUnordered)
    end
    ------------------------------------------------------------------------------------------------
    -- ordered global view set
    local function GlobalViewed()
        if not ts.globalViewed then
            ts.globalViewed = {}
        end

        return instance(ts.globalViewed)
    end

    ------------------------------------------------------------------------------------------------
    -- create ordered temporary canditates
    local function Candidates()
        if not ts.candidates then
            ts.candidates = {}
        end

        return instance(ts.candidates)
    end

    ------------------------------------------------------------------------------------------------
    -- create ordered temporary viewed set
    local function Viewed()
        if not ts.viewed then
            ts.viewed = {}
        end

        return instance(ts.viewed)
    end

    ------------------------------------------------------------------------------------------------
    -- create temporary viewed set
    local function VisitedUnordered()
        if not ts.visitedUnordered then
            ts.visitedUnordered = {}
        end

        return instance(ts.visitedUnordered)
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
        local candidates,viewed, visitedUnordered = Candidates(),Viewed(),VisitedUnordered()
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
            lowerBound = kDistance(viewed, worldSize);
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
                    globalUnordered[node.name] = candidate.distance
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

        local nodes = Nodes()
        local globalUnordered,global = GlobalUnorderedViewSet(),GlobalViewed()

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
        --temp:begin()
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
        --print("adding",value,"as","#"..name)

        local viewed = search(self,value)
        nodes[name] = query
        stats.count = stats.count + 1
        local toadd = nodes[name] -- use the persisted version
        local i = 0
        for other,value in pairs(viewed) do
            if i >= worldSize then
                break
            end
            i = i + 1
            -- two way friends
            local pvalue = nodes[value.name] -- reaquire from storage
            toadd.friends[pvalue.name] = pvalue
            pvalue.friends[name] = toadd
        end
        --temp:rollback()
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


return Create