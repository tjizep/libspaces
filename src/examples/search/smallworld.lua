----------------------------------------------------------------------------------------------------
-- a persisted metricized navigable small world index in lua and spaces
-- lua/spaces/persisted version by Christiaan Pretorius chrisep2@gmail.com
-- based on java code by Alexander Ponomarenko aponom84@gmail.com
-- NB: requires caller/user to seed lua internal pseudo random generator for sufficient randomness
-- This version uses ordred subgraphs of friends which allows the accelerated convergence to a
-- minimum which can improve k-nn search times
----------------------------------------------------------------------------------------------------
--local inspect = require "inspect_meta"
--local spaces =
--require "spaces"
----------------------------------------------------------------------------------------------------
-- creates a new sw object for use
-- worldSize is the maximum friends that will be added to each node at a time
-- the friends list size can definitely grow larger
-- sample size is BF over priority queue searches that will be done to approach
-- the closest node to the query parameter
----------------------------------------------------------------------------------------------------
__seed = 1
local function get_seed()
    __seed = __seed + 1
    return __seed
end
local function Create(groot,worldSize,sampleSize,metricFunction)
    ------------------------------------------------------------------------------------------------
    -- f.y.i.: 'groot' is the afrikaans word for large and not the character in that movie
    -- although a sentient tree would be a cute analogy
    -- spaces.storage(storage) should be called before this function
    ------------------------------------------------------------------------------------------------
    ---
    --- closure variables initialization
    ---
    ------------------------------------------------------------------------------------------------

    local ival =1
    local seed = 0
    --groot.temp = {}
    local tname = "_["..os.clock()..get_seed().."]_"
    local temp = spaces.open(tname)
    local ts = temp:open()
    local function metricSeed()
        -- function used for emulating a priority queue - because spaces are unique ordered sets
        seed = seed + 1
        return seed /1e14
    end
    local function metric(a,b)
        return metricFunction(a,b) + metricSeed()
    end
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
    local function instance(val,name)
        if name then
            print("instance",name,c)
        end
        val[0] = {}
        return val[0]
    end
    ------------------------------------------------------------------------------------------------
    -- initialize temporary global view sets
    local function GlobalUnorderedViewSet()
        if ts.globalUnordered == nil then
            ts.globalUnordered = {}
        end

        return {} --instance(ts.globalUnordered)
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
        local r = ts.candidates

        return r
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

        return {} --instance(ts.visitedUnordered)
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
        if set == nil then
            return 0
        end
        return set():key(math.min(k,#set))
    end

    ------------------------------------------------------------------------------------------------
    --- Per Instance functions, they have a self in front
    ------------------------------------------------------------------------------------------------
    -- search nodes
    -- the similarity with dijkstras algorithm is neither coincidental nor accidental
    local function searchNodes(query, entry, globalUnordered)

        local lowerBound = 0
        local candidates,viewed, visitedUnordered = Candidates(),Viewed(),VisitedUnordered()
        local distance = metric(query,entry.value)
        local candidate = { name=entry.name, value=entry.value, friends=entry.friends,distance=distance }
        globalUnordered[entry.name] = candidate

        visitedUnordered[entry.name] = candidate
        candidates[distance] = candidate --- the random starting point
        viewed[distance] = candidate

        while not candidates():empty() do
            local current = candidates():firstValue()
            --- find the k lower bound of the current ordered viewed set
            lowerBound = kDistance(viewed, worldSize);
            candidates[candidates():firstKey()] = nil

            if current.distance > lowerBound then
                break
            end
            --- remember which have been visited for this instance of the k search
            visitedUnordered[current.name] = current
            --- look at the friends of the current candidate
            --- attempt to find a closer friend of friends

            ft = os.clock()
            local fx = 1
            --- NB
            --- this is a modification on the original algorithm
            --- only the first K + log(N) friends are used since
            --- each friends node is also a priority queue
            --- which means the closest friends are viewed first
            --- this seems to converge to a minimum quicker
            --- than the original algorithm for spaces anyway
            local tf = worldSize*1.5 + math.log(Stats().count)--worldSize*1.5
            local tfdist,tdist = 0,0
            for fdist,node in pairs(current.friends) do --- visit nodes in order of closeness
                local name = node.name
                --- do not visit the node again
                if globalUnordered[name] == nil then
                    local value = node.value
                    dt = os.clock()
                    local dist = metric(query,value)
                    tfdist = tfdist + fdist
                    tdist = tdist + dist
                    local candidate = { name=name, value=value, friends=node.friends,distance = dist }
                    --- remember the closest nodes for all k instances of k search
                    globalUnordered[name] = dist
                    --- add candidates in order of closeness
                    candidates[dist] = candidate
                    --- add viewed set in order of closeness
                    viewed[dist] = candidate
                    fx = fx + 1
                end
                if fx > tf then
                   -- if math.abs(tfdist-tdist) > tdist*0.2 then
                    --else
                        break
                   -- end
                end
            end
        end

        for k,v in pairs(visitedUnordered) do
            globalUnordered[k] = v
        end

        return viewed
    end

    ------------------------------------------------------------------------------------------------
    -- finds nodes closest to query parameter according to distance function
    local function search (query)
        local gcs = os.clock()
        collectgarbage("collect") -- or we will get an invalid reference count error
        temp:rollback()
        temp:write()
        local nodes = Nodes()
        local stats = Stats()
        local globalUnordered,global = GlobalUnorderedViewSet(),GlobalViewed()
        local gs = 0
        local totalViewed = 0
        local ts = os.clock()
        for i=1,sampleSize do

            local rn = getRandomNode(nodes)
            local viewed = searchNodes(query,rn,globalUnordered)

            for k,v in pairs(viewed) do
                global[k] = v
                totalViewed = totalViewed + 1
            end
        end

        --print(totalViewed,"result ratio",(totalViewed/sampleSize)/stats.count,1/gs,"qps")

        return global
    end

    ------------------------------------------------------------------------------------------------
    -- add a node
    -- name should be unique
    local function add(value)

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

        local viewed = search(value)
        nodes[name] = query
        stats.count = stats.count + 1
        local toadd = nodes[name] -- use the persisted version
        local i = 0
        for dist,value in pairs(viewed) do
            if i >= worldSize then
                break
            end
            i = i + 1
            -- two way friends
            local pvalue = nodes[value.name] -- reaquire from storage
            -- the original smallworld algorithm did not add friends
            -- in a priority queue - its possible with spaces
            -- since the sub graphs are ordered by name
            toadd.friends[dist] = pvalue
            pvalue.friends[dist] = toadd
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
local function CreateSegmented(groot,worldSize,sampleSize,metricFunction)
    if groot.worlds == nil then
        groot.worlds = {}
        groot.stats = {size=0,count=0}
    end
    local tname = "_[]["..os.clock()..get_seed().."]_"
    local temp = spaces.open(tname)
    local ts = temp:open()

    local stats = groot.stats
    local worlds = groot.worlds
    local instances = {}
    local seed = 0
    ------------------------------------------------------------------------------------------------
    -- returns a persisted world based on the item
    ------------------------------------------------------------------------------------------------
    local function metricSeed()
        -- function used for emulating a priority queue - because spaces are unique ordered sets
        seed = seed + 1
        return seed /1e8
    end
    ------------------------------------------------------------------------------------------------
    -- instantiates results for new search
    ------------------------------------------------------------------------------------------------
    local function Results()
        ts.results = {}
        return ts.results
    end
    ------------------------------------------------------------------------------------------------
    -- returns a persisted world based on the item
    ------------------------------------------------------------------------------------------------
    local function World(which)

        if worlds[which] == nil then
            worlds[which] = {}
        end
        return worlds[which]
    end
    ------------------------------------------------------------------------------------------------
    -- returns a world index (referred to as which) based on x
    ------------------------------------------------------------------------------------------------
    local function Select(x)
        -- you may modify this parameter to create multithreaded searches
        return math.floor(x / 500000) + 1
    end
    ------------------------------------------------------------------------------------------------
    -- returns a world instance based on which
    ------------------------------------------------------------------------------------------------

    local function Instance(which)
        if instances[which] == nil then
            instances[which] = Create(World(which),worldSize,sampleSize,metricFunction)
        end
        return instances[which]

    end
    ------------------------------------------------------------------------------------------------
    -- find values based on suplied metric
    ------------------------------------------------------------------------------------------------
    local function search(self,query)
        collectgarbage("collect") -- or we will get an invalid reference count error
        temp:rollback()
        temp:write()
        local results = Results()
        local id = 1
        for which,world in pairs(worlds) do

            local found = Instance(which).search(query)
            for k,v in pairs(found) do
                results[k+id/1e12] = v
                id = id + 1
            end

        end
        return results

    end
    ------------------------------------------------------------------------------------------------
    -- add a value using supplied metric
    ------------------------------------------------------------------------------------------------
    local function add(self,value)
        Instance(Select(stats.count)).add(value)
        stats.count = stats.count + 1

    end
    ------------------------------------------------------------------------------------------------
    -- the exposed library object
    ------------------------------------------------------------------------------------------------
    local snsw = {
        search = search,
        add = add
    }
    return snsw
end

return CreateSegmented