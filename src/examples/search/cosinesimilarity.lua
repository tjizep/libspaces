-----------------------------------------------------------------------------------------------
-- cosine similarity if two vectors
-----------------------------------------------------------------------------------------------
local function cosim(a,b)

    local top = 0.0
    local bA = 0.0
    local bB = 0.0
    for i,ai in ipairs(a) do
        local bi = b[i] or 0
        top = top + ai*bi
        bA = bA + ai*ai
        bB = bB + bi*bi
    end
    bA = math.sqrt(bA)
    bB = math.sqrt(bB)
    return top/(bA*bB)
end

return cosim