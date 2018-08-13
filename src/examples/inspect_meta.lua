local function inspect(t)
    local out = {}
    local visited = {}
    local function put(arg)
        if type(arg) == "table" then
            for i,v in ipairs(arg) do
                table.insert(out,v)
            end
        else
            table.insert(out,arg)
        end
    end
    local function line(arg)
        put(arg)
        table.insert(out,"\n")

    end
    local function makespace(level)
        local spacer = ""
        for l=1,level do
            spacer = spacer.."\t"
        end
        return spacer
    end
    local function inspectv(v,level)
        if type(v) == "table" then

        elseif type(v) =="string" then
            put({"\"",v,"\""})
        elseif type(v) == "nil" then
            put({"nil"})
        else
            put({v})
        end
    end

    local function inspectt(t,level)
        if type(t) == "table" then
            if visited[t] ~= nil then
                put({"..."})
                return
            end
            visited[t] = level

            local spacer = makespace(level)

            local i = 1
            local lt = #t
            if lt == 0 then
                put({"{}"})
            else
                visited[t] = level

                line({spacer})
                line({spacer,"{"})
                for k,v in pairs(t) do

                    if type(k) == "number" then
                        put({spacer,"\t[",k,"] = "})
                    else
                        put({spacer,"\t",k," = "})
                    end

                    inspectt(v,level+1)
                    if i < lt then
                        put({","})
                    end
                    line("")

                    i = i + 1
                end

                put({spacer,"}"})
            end
            visited[t] = nil

        else
            inspectv(t,level)
        end
    end
    inspectt(t,0)

    return table.concat(out)
end

return inspect