require("spaces")
spaces.storage("tutorial")

local s = spaces.open()
-- assign data
s.cities =  {
    London  = { population = 8615246, area=10000 },
    Berlin  = { population = 3517424, area=12220 },
    Madrid  = { population = 3165235, area=12220 },
    Rome    = { population = 2870528, area=12220 },
    Pretoria= { population = 2248000, area=12220 }
}
-- persist atomically
spaces.commit()

-- print some value
print(s.cities.London.population)

--iterate
for k,v in pairs(s.cities) do
    print(k, s.cities[k].population)
end

for k,v in pairs(s.cities) do
    print(k, v.population, v.area)
end