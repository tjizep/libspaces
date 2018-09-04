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
-- worldSize is the maximum friends a that will be added to each node at a time
-- the friends list size can definitely grow larger
-- sample size is BF over priority queue searches that will be done to approach
-- the closest node to the query parameter
----------------------------------------------------------------------------------------------------
local function Create(groot,worldSize,sampleSize,metricFunction)
    ------------------------------------------------------------------------------------------------
    -- f.y.i.: 'groot' is the afrikaans word for large and not the character in that movie
    --
    -- spaces.storage(storage) should be called before this function
    ------------------------------------------------------------------------------------------------
    ---
    --- closure variables initialization
    ---
    ------------------------------------------------------------------------------------------------
    local ival =1
    local seed = 0
    --groot.temp = {}
    local temp = spaces.open("temp"..os.clock())
    print("opening")
    local ts = temp:open()
    local function metricSeed()
        -- function used for emulating a priority queue - because spaces are unique ordered sets
        seed = seed + 1
        return seed /1e8
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
            --print("create globalUnordered")
            ts.globalUnordered = {}


        end

        return {} --instance(ts.globalUnordered)
    end
    ------------------------------------------------------------------------------------------------
    -- ordered global view set
    local function GlobalViewed()
        if not ts.globalViewed then
            --print("create globalViewed")
            ts.globalViewed = {}
        end

        return instance(ts.globalViewed)
    end

    ------------------------------------------------------------------------------------------------
    -- create ordered temporary canditates
    local function Candidates()
        if not ts.candidates then
            --print("create candidates")
            ts.candidates = {}
        end

        return instance(ts.candidates)
    end

    ------------------------------------------------------------------------------------------------
    -- create ordered temporary viewed set
    local function Viewed()

        if not ts.viewed then
            --print("create viewed")
            ts.viewed = {}
        end
        return instance(ts.viewed)
    end

    ------------------------------------------------------------------------------------------------
    -- create temporary viewed set
    local function VisitedUnordered()
        if not ts.visitedUnordered then
            --print("create visitedUnordered")
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

        local doDebug = false
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
                    local dist = metric(query,node.value)
                    local candidate = { name=node.name, value=node.value, friends=node.friends,distance = dist }
                    --- remember the closest nodes for all k instances of k search
                    globalUnordered[node.name] = dist
                    --- add candidates in order of closeness
                    candidates[dist] = candidate
                    --- add viewed set in order of closeness
                    viewed[dist] = candidate
                    if doDebug then
                        local tn = viewed[dist].name
                        local tv = viewed[dist].value
                        print(query,"->viewed (n="..#viewed..")",type(tn),tn,tv)
                    end
                end
            end

        end
        if doDebug then
            print("viewed count "..#viewed)
            for k,v in pairs(viewed) do
                print(k,type(v.name))
            end
        end
        for k,v in pairs(visitedUnordered) do
            globalUnordered[k] = v
        end

        return viewed -- visitedUnordered
    end

    ------------------------------------------------------------------------------------------------
    -- finds nodes closest to query parameter according to distance function
    local function search (self, query)
        collectgarbage("collect") -- or we will get an invalid reference count error
        temp:rollback()
        temp:write()
        local nodes = Nodes()
        local globalUnordered,global = GlobalUnorderedViewSet(),GlobalViewed()
        local added = {}
        for i=1,sampleSize do

            local viewed = searchNodes(query,getRandomNode(nodes),globalUnordered)
            for k,v in pairs(viewed) do
                global[k] = v
                added[v.name] = 1
            end
        end
        return global
    end

    ------------------------------------------------------------------------------------------------
    -- add a node
    -- name should be unique
    local function add(self, value)

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
        for _,value in pairs(viewed) do
            if i >= worldSize then
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


return Create