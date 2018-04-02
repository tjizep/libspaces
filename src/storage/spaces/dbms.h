#pragma once
#include <string>
#include <iostream>
#include <storage/spaces/key.h>
#include <stx/btree_set.h>
#include <abstracted_storage.h>
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
		void commit() {
			set.flush_buffers();
			if (this->storage.is_transacted())
				this->storage.commit();

			this->storage.begin(true);
		}
	};
}