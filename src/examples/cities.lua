require "spaces"
local inspect = require('inspect_meta')
local jp = require('jsonpath')
spaces.storage("cities") -- puts data in the cities subdirectory
local s = spaces.open(); -- starts a transaction automatically

if s.cities==nil then
    local cities = {
        { name = 'London', population = 8615246 },
        { name = 'Berlin', population = 3517424 },
        { name = 'Madrid', population = 3165235 },
        { name = 'Rome',   population = 2870528 }
    }
    local names = jp.query(cities, '$..name')
    print("from lua",inspect(names))
    s.cities = cities
    s.stores = {
        store = {
            bicycle = {
                color = 'red',
                price = 19.95
            },
            book = {
                {
                    category = 'reference',
                    author = 'Nigel Rees',
                    title = 'Sayings of the Century',
                    price = 8.95
                }, {
                    category = 'fiction',
                    author = 'Evelyn Waugh',
                    title = 'Sword of Honour',
                    price = 12.99
                }, {
                    category = 'fiction',
                    author = 'Herman Melville',
                    title = 'Moby Dick',
                    isbn = '0-553-21311-3',
                    price = 8.99
                }, {
                    category = 'fiction',
                    author = 'J. R. R. Tolkien',
                    title = 'The Lord of the Rings',
                    isbn = '0-395-19395-8',
                    price = 22.99
                }
            }
        }
    }

    spaces.commit()
else
    print("data from storage")
end
-- names of cities
local names = jp.query(s.cities, '$..name')
print("city names",inspect(names))
-- The price of everything in the store
local prices = jp.query(s.stores,'$.store..price')
print("prices",inspect(prices))
-- authors of books
local authors = jp.query(s.stores,'$.store.book[*].author')
print("authors",inspect(authors))
-- cheap fiction books
local cheapFiction = jp.query(s.stores,'$..book[?(@.price<10 && @.category=="fiction")]')
print("cheapFiction",inspect(cheapFiction))
-- mid price fiction books titles
local medFiction= jp.query(s.stores,'$..book[?(@.price>=10 && @.price<300 && @.category=="fiction")]')
print("medFiction",inspect(medFiction))
-- cheap non fiction books
local cheapNonFiction = jp.query(s.stores,'$.store.book[?(@.price<10 && @.category!="fiction")]')
print("cheapNonFiction",inspect(cheapNonFiction))

