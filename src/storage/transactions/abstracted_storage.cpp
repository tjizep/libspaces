#include "abstracted_storage.h"
#include <storage/spaces/dbms.h>
#include <thread>


struct statics
{
	statics(){
		this->instances = std::make_shared<stored::_AlocationsMap>();
		this->db_mem_mgr = std::make_shared<spaces::dbms_memmanager>();

	}
	Poco::Mutex m;
	std::shared_ptr<stored::_AlocationsMap> instances;
	std::shared_ptr<spaces::dbms> writer;
	std::shared_ptr<spaces::dbms_memmanager> db_mem_mgr;
	~statics(){
		//this->writer = nullptr;
		this->db_mem_mgr = nullptr;
		this->instances = nullptr;
	}
};
static statics variables;

namespace nst = ::stx::storage;
namespace stx{
	namespace storage{
		bool storage_debugging = false;
		bool storage_info = false;
	}
};




spaces::dbms::ptr spaces::create_reader() {
	auto r = std::make_shared<dbms>(STORAGE_NAME, true);
	//variables.db_mem_mgr->add(r.get());
	return r;
}
spaces::dbms::ptr  spaces::get_writer(){
	if(writer == nullptr){
		writer = std::make_shared<spaces::dbms>(STORAGE_NAME,false);
		//variables.db_mem_mgr->add(writer.get());
	}
	return writer;
}
namespace stored{

	_Allocations* _get_abstracted_storage(const std::string& name){

		_Allocations* r = NULL;
		r = variables.instances->operator[](name);
		if(r == NULL){
			r = new _Allocations( std::make_shared<_BaseAllocator>( stx::storage::default_name_factory(name.c_str())));
			variables.instances->operator[](name) = r;
		}

		return r;
	}
	bool erase_abstracted_storage(std::string name){

		nst::synchronized ll(variables.m);
		_Allocations* r = NULL;
		_AlocationsMap::iterator s = variables.instances->find(name);
		if(s == variables.instances->end()){
			inf_print("cannot erase: storage [%s] not found",name.c_str());

			return true;
		}
		r = (*s).second;
		if(r == nullptr){
			inf_print("cannot erase: storage [%s] unloaded",name.c_str());
			return true;
		}

		if(r->is_idle()){

			r->set_recovery(false);
			delete r;
			variables.instances->erase(name);
			inf_print("storage [%s] erased",name.c_str());
			return true;
		}else{
			inf_print("the storage '%s' is not idle and could not be erased",name.c_str());
		}

		return false;
	}
	_Allocations* get_abstracted_storage(std::string name){
		_Allocations* r = nullptr;
		{

			nst::synchronized ll(variables.m);
			if (variables.instances->empty())
				nst::journal::get_instance().recover();
			r = variables.instances->operator[](name);
			if (r == NULL) {
				r = new _Allocations(
						std::make_shared<_BaseAllocator>(stx::storage::default_name_factory(name)));
				///r->set_readahead(true);
				variables.instances->operator[](name) = r;
			}
			r->set_recovery(false);
			r->engage();
		}
		return r;
	}
	void reduce_aged(){
		nst::synchronized ll(variables.m);
		nst::u64 reduced = 0;
		for(_AlocationsMap::iterator a = variables.instances->begin(); a != variables.instances->end(); ++a){
			//if(buffer_allocation_pool.is_near_depleted()){ //
				if((*a).second->get_age() > 15000 && (*a).second->transactions_away() <= 1){
					//(*a).second->reduce();
					//(*a).second->touch();
					//reduced++;
				}
			//}
		}
		if(reduced)
			inf_print("reduced %llu aged block storages",(nst::fi64)reduced);
	}
	void reduce_all(){

		nst::synchronized ll(variables.m);

		for(_AlocationsMap::iterator a = variables.instances->begin(); a != variables.instances->end(); ++a){
			if(buffer_allocation_pool.is_near_depleted()){
				(*a).second->reduce();
			}

		}
	}
}; //stored

