#include "pool.h"
namespace stx{
namespace storage{
namespace allocation{
    pool_shared* get_shared(){
        static pool_shared shared;
        return &shared;
    }
}
}
}
