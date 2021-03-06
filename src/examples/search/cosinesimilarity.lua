-----------------------------------------------------------------------------------------------
-- cosine similarity if two vectors
-----------------------------------------------------------------------------------------------
local function cosim(a,b)

    local top = 0.0
    local A = 0.0
    local B = 0.0
    local ai,bi
    for i = 1, #a do
        ai = a[i]
        bi = b[i]
        top = top + ai*bi
        A = A + ai*ai
        B = B + bi*bi
    end
    return top/math.sqrt(A*B)
end

return cosim