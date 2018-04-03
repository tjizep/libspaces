#pragma once
#include <string>
#include <iostream>
#include <storage/spaces/key.h>
#include <stx/btree_set.h>
#include <abstracted_storage.h>
namespace stx {
    namespace storage {
        extern long long total_use;
    }
}
namespace spaces{
	class dbms {
	public:

		stored::abstracted_storage storage;
		typedef stx::btree_set< key, stored::abstracted_storage, std::less<key>> _Set; 
	private:
		_Set set;
	public:
		dbms(const std::string &name) : storage(name), set(storage) {
			storage.begin(true);
			allocation_pool.set_max_pool_size(1024*1024*1024*6ull);
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
		void begin() {
			if (!this->storage.is_transacted()) {				
				stored::abstracted_tx_begin(false, false, storage, set);
			}
				
		}
		void check_resources(){

		}
		void commit() {
			set.flush_buffers();
			if (this->storage.is_transacted())
				this->storage.commit();

			this->storage.begin(true);
		}
	};
}