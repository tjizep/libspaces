****LIB SPACES****
-----------------

****TUTORIAL****

To begin we can setup any environment that has a lua interpreter and start from there. Please ensure that you have the 
cpath variable configured to see lib spaces.

**Opening a database**

    require("spaces")

This will create the spaces user data type in lua

Optionally we can assign a storage directory to store all the data files in. If this is not assigned then the current directory will be used
    
    spaces.storage("tutorial")

From here we can open the root node

    local s = spaces.open() 

This will create the object 's' which is the the root of the database.

Now it's possible to create some data with a simple statement

    s.cities =  {
                    London  = { population = 8615246, area=10000 },
                    Berlin  = { population = 3517424, area=12220 },
                    Madrid  = { population = 3165235, area=12220 },
                    Rome    = { population = 2870528, area=12220 },
                    Pretoria= { population = 2248000, area=12220 }
                } 
    
    
    
     
This data is not saved yet...

    spaces.commit()
 
 Will save it to disk, tcp packets, pigeons etc.
 
 Printing the population of 'London'...
 
    print(s.cities.London.population)
 
 Iterating through all the cities and print their population
    
    for k,v in pairs(s.cities) do
        print(k, s.cities[k].population)
    end

The code for this tutorial can be found [here](src/examples/tutorial.lua) 