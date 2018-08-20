#include "abstracted_storage.h"
#include <storage/spaces/dbms.h>
#include <thread>


class statics
{
public:
	typedef spaces::dbms::ptr dbms_ptr;
private:
	std::shared_ptr<spaces::dbms_memmanager> db_mem_mgr;
	Poco::Mutex mwriters;
	typedef rabbit::unordered_map<std::string,spaces::dbms::ptr> _WriterMap;
	_WriterMap writers;
public:
	statics(){
	}
	void close() {
		this->writers.clear();
		this->db_mem_mgr = nullptr;
		this->instances = nullptr;
	}
	void open() {
		this->instances = std::make_shared<stored::_AlocationsMap>();
		this->db_mem_mgr = std::make_shared<spaces::dbms_memmanager>();
	}
	spaces::dbms::ptr  get_writer(const std::string &name) {
		dbms_ptr writer;
		nst::synchronized sync(mwriters);
		auto w = writers.find(name);
		if (w == writers.end()) {

			writer = get_db_mem_mgr()->add(name, false);
			writers[name] = writer;
		}else{
			writer = w->second;
		}
		return writer;
	}
	std::shared_ptr<spaces::dbms_memmanager>& get_db_mem_mgr() {
		if (this->db_mem_mgr == nullptr) {
			this->db_mem_mgr = std::make_shared<spaces::dbms_memmanager>();
		}
		return this->db_mem_mgr;
	}
	Poco::Mutex m;
	std::shared_ptr<stored::_AlocationsMap> instances;
;
	~statics(){
		
	}
};

static statics variables;
static thread_local std::string _t_str;
void start_storage() {
	variables.open();
}
namespace stored{
	void stop() {
		variables.close();
	}
}

namespace nst = ::stx::storage;
namespace stx{
	namespace storage{
		const char * tostring(const version_type& v) {
			_t_str = v.toString();
			return _t_str.c_str();
		}
		bool storage_debugging = false;
		bool storage_info = false;
	}
};




spaces::dbms::ptr spaces::create_reader(const std::string& name) {
	
	auto r = variables.get_db_mem_mgr()->add(name, true);
	return r;
}
spaces::dbms::ptr  spaces::get_writer(const std::string& name ){
	return variables.get_writer(name);
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

