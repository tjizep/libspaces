#pragma once
#include <string>
#include <iostream>
#include <storage/spaces/key.h>
#include <stx/btree_map.h>
#include <storage/transactions/abstracted_storage.h>
#define STORAGE_NAME "spaces.data"

namespace stx {
    namespace storage {
        extern long long total_use;
    }
}
namespace spaces{

	class dbms {
	public:

		stored::abstracted_storage storage;
		typedef stx::btree_map<key, record, stored::abstracted_storage, std::less<key>> _Set;
	private:
		Poco::FastMutex lock;
		_Set set;
		nst::i64 id;
		bool is_reader;
		static const nst::stream_address ID_ADDRESS = 8;
	public:
		bool reader() const {
			return  this->is_reader;
		}
		dbms(const std::string &name,bool is_reader) : storage(name), set(storage), id(1) {
            
			allocation_pool.set_max_pool_size(1024*1024*1024*10ull);
            storage.rollback();
		}
		~dbms() {
			
			inf_print("stopping dbms...%s", storage.get_name().c_str());		
			storage.rollback();
			inf_print("ok %s", storage.get_name().c_str());			
		}
		const std::string& get_name(){
			return storage.get_name();
		}

		_Set& get_set() {
			return set;
		}
		nst::u64 gen_id(){
		    return id++;
		}
		void begin() {
			if (!this->storage.is_transacted()) {

			    lock.lock();

				stored::abstracted_tx_begin(is_reader, false, storage, set);
                if(!storage.get_boot_value(id,ID_ADDRESS)){
                    id = 1;
                }
			}
		}
		void rollback(){
            if (this->storage.is_transacted()) {
                this->storage.rollback();
            }
		}

		void check_resources(){

		}
		void commit() {

			if (this->storage.is_transacted()){
				set.flush_buffers();

				try{
					if(this->is_reader){
						this->storage.rollback();
					}else{
						storage.set_boot_value(id, ID_ADDRESS);
						this->storage.commit();
						nst::journal::get_instance().synch();
					}
				}catch (...){
					err_print("error during commit");
				}


                lock.unlock();
			}

		}
	};
	extern std::shared_ptr<dbms> get_writer();
	static std::shared_ptr<dbms> create_reader(){
		return std::make_shared<dbms>(STORAGE_NAME,true);
	}
}