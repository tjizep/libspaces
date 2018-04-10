//
// Created by pirow on 2018/04/09.
//

#ifndef SPACES_REPLICATION_H
#define SPACES_REPLICATION_H
#include <storage/spaces/dbms.h>
namespace nst = stx::storage;
namespace spaces{
    class replication_server{
    private:
        nst::u16 port;
    public:
        replication_server(nst::u16 port);
        void run();
        ~replication_server();
    };
}


#endif //SPACES_REPLICATION_H
