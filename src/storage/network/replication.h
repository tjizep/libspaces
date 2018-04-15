//
// Created by pirow on 2018/04/09.
//

#ifndef SPACES_REPLICATION_H
#define SPACES_REPLICATION_H
#include <storage/transactions/abstracted_storage.h>
#include <rpc/client.h>
#include <rpc/server.h>
namespace nst = stx::storage;
namespace spaces{
    static const nst::u16 DEFAULT_PORT = 16003;
    class block_replication_server{
    private:
        nst::u16 port;
    public:
        block_replication_server(nst::u16 port);
        void run();
        ~block_replication_server();
    };
    class block_replication_client{
    private:
        std::string address;
        typedef std::shared_ptr<rpc::client> client_ptr;
        client_ptr remote;
    public:
        block_replication_client();
        void set_address(const std::string& address);
        bool is_open() const ;
        void begin(bool is_read);
        void commit();
        void rollback();
        void open(bool is_new,const std::string& name);
        void store(nst::u64 address, const nst::buffer_type& data);
        bool get(nst::version_type& version, nst::buffer_type& data, nst::u64 address);
        bool contains(nst::u64 address);
        void close();
        nst::u64 max_block_address() const;
        ~block_replication_client();
    };
}


#endif //SPACES_REPLICATION_H
