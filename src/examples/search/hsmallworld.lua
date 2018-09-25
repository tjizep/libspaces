----------------------------------------------------------------------------------------------------
-- The hierarchical or layered navigable small world code is based on this well known paper
-- https://arxiv.org/pdf/1603.09320.pdf
-- with more better pseudo code and algorithmic explanation at
-- https://pdfs.semanticscholar.org/699a/2e3b653c69aff5cf7a9923793b974f8ca164.pdf
-- the hnsw also requires randomness to select a layer
----------------------------------------------------------------------------------------------------

--local inspect = require "inspect_meta"

local spaces = require "spaces"
----------------------------------------------------------------------------------------------------
-- creates a new sw object for use
-- worldSize is the maximum friends a that will be added to each node per layer in the hierarchy
--
-- sampleSize can be some integer value like 200

----------------------------------------------------------------------------------------------------
local function Create(groot,worldSize,creationSampleSize,metricFunction)
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
    local tname = "_["..os.clock().."]_"
    local temp = spaces.open(tname)
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
        if groot.hindex==nil then
            groot.hindex = {nodes={}}
        end
        return groot.hindex.nodes
    end
    local function Stats()
        if groot.hstats==nil then
            groot.hstats = {count=0,maxLayer=0,entry={}}
        else
            seed = groot.hstats.count
        end
        return groot.hstats
    end
    ------------------------------------------------------------------------------------------------
    -- alloc temp instance
    local function instance(val,name)
        if name then
            print("instance",name,c)
        end
        ival = ival + 1
        val[ival] = {}
        return val[ival]
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

        return {} --instance(ts.visitedUnordered)
    end
    ------------------------------------------------------------------------------------------------
    -- return a random node
    local function toLayer(node,layer)
        if node.friends[layer] == nil then
            node.friends[layer] = {}
        end
        return node.friends[layer]

    end
    ------------------------------------------------------------------------------------------------
    -- return a random node
    local function getRandomNode(nodes)
        local randomKey = math.random(1,Stats().count)
        local node = nodes[randomKey]
        return {name=randomKey,value=node.value,friends=node.friends}   -- copy values
    end

    ------------------------------------------------------------------------------------------------
    --- Per Instance functions, they have a self in front
    ------------------------------------------------------------------------------------------------

    ------------------------------------------------------------------------------------------------
    -- M is the constant at which layers are abandonded in favour of deeper ones
    -- or the maximum friend connections a node can have per layer

    local M = 20

    ------------------------------------------------------------------------------------------------
    -- finds nodes closest to query parameter according to distance function using the hierarchical
    -- navigable small worlds algorithm
    -- the 'layer' argument smells like recursion waiting to happen

    local function searchLayer(query, entries, sampleSize, layer)
        local candidates,viewed, visitedUnordered = Candidates(),Viewed(),VisitedUnordered()
        for _,entry in pairs(entries) do
            local distance = metric(query,entry.value)
            local candidate = { name=entry.name, value=entry.value, friends=entry.friends,distance=distance }

            visitedUnordered[entry.name] = candidate
            candidates[distance] = candidate --- the starting point
            viewed[distance] = candidate
        end
        local c = candidates()
        while not c:empty() do
            local current = c:firstValue()
            local v = viewed()
            candidates[c:firstKey()] = nil --- equivalent to p queue pop
            if metric(current.value, query) > metric(query, v:lastValue().value) then
                break
            end

            for _,node in pairs(toLayer(current,layer)) do
                if visitedUnordered[node.name] == nil then
                    visitedUnordered[node.name] = node
                    local dist = metric(query,node.value)
                    if #viewed < sampleSize or dist < metric(v:lastValue().value, query) then
                        candidates[dist] = node --- the greedy part
                        viewed[dist] = node
                        if #viewed > sampleSize then
                            viewed[viewed():lastKey()] = nil --- prune most distant element
                        end
                    end
                end
            end
            c = candidates()
        end
        return viewed
    end
    ------------------------------------------------------------------------------------------------
    -- logarithm base 10
    --
    local function log(x)
        return math.log(x)
    end
    ------------------------------------------------------------------------------------------------
    -- random variable from x to y
    --
    local function random(x,y)
        return math.random(x,y)
    end
    ------------------------------------------------------------------------------------------------
    -- return integer
    --
    local function floor(x)
        return math.floor(x)
    end
    ------------------------------------------------------------------------------------------------
    -- return minimum or smallest
    --
    local function min(x,y)
        return math.min(x,y)
    end
    ------------------------------------------------------------------------------------------------
    -- insert node into layered smallworld graph
    --
    local function add(self,value)
        collectgarbage("collect") -- or we will get an invalid reference count error
        temp:rollback()
        temp:write()

        local nodes = Nodes()
        local stats = Stats()
        local maxLayer = stats.maxLayer
        local name = stats.count + 1
        local query = { name=name,value=value,friends={} }
        if not nodes[name] == nil then
            error("node "..name.." already exists ")
        end

        if nodes():empty() then
            stats.count = stats.count + 1
            stats.entry = name
            nodes[name] = query
            return
        end

        local levelMult = 1/log(M)
        local entry = nodes[stats.entry]
        local result = {[1]=entry }
        local rvar = random(0,1e6)/1e6
        local level =  floor(-log(rvar)*12)

        for layer = maxLayer, level - 1, -1 do
            result = searchLayer(value,result,1,layer)
        end
        nodes[name] = query
        stats.count = stats.count + 1
        local toadd = nodes[name] --- use the persisted version
        local start = result
        for layer=min(maxLayer, level),0,-1 do
            result = searchLayer(value,start,creationSampleSize,layer)
            local n = 0
            for distance,value in pairs(result) do
                local pvalue = nodes[value.name] --- reaquire from storage

                toLayer(toadd,layer)
                toLayer(pvalue,layer)

                toadd.friends[layer][distance] = pvalue
                pvalue.friends[layer][distance] = toadd
                local shrinkme = pvalue.friends[layer]
                while #shrinkme > M do
                    shrinkme[shrinkme():lastKey()] = nil
                end

                n = n + 1
                if n == M then
                    break
                end
            end

        end
        if level > maxLayer then
            stats.entry = name
            stats.maxLayer = level
        end


    end
    ------------------------------------------------------------------------------------------------
    -- search layered smallworld graph
    --
    local function search(self,value,ef)
        collectgarbage("collect") -- or we will get an invalid reference count error
        temp:rollback()
        temp:write()
        local nodes = Nodes()
        local stats = Stats()
        local maxLayer = stats.maxLayer
        local result = {[1]= nodes[stats.entry]}

        for layer=maxLayer,1,-1 do
            result = searchLayer(value,result,1,layer)
        end
        result = searchLayer(value,result,ef or 16,0)
        return result --- let the user decide how many to use
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