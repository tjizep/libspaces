#include <storage/interface/space_lua.h>
#include <stx/storage/basic_storage.h>
#include <storage/transactions/transactional_storage.h>
#include <string>


namespace nst = stx::storage;
nst::allocation::pool allocation_pool(2 * 1024ll * 1024ll * 1024ll);
nst::allocation::pool buffer_allocation_pool(2 * 1024ll * 1024ll * 1024ll);

std::string contact_points = "";
char	block_read_ahead = 0;
char	use_internal_pool = 0;
nst::u64 max_mem_use = 1024 * 1024 * 1024;
nst::u64 store_journal_size = 0;
nst::u64 store_journal_lower_max = 2 * 1024ll * 1024ll * 1024ll;

nst::u64 store_max_mem_use;
nst::u64 store_current_mem_use;
ptrdiff_t btree_totl_used = 0;
ptrdiff_t btree_totl_instances = 0;
bool stx::memory_low_state = false;
bool stx::memory_mark_state = false;
std::string data_directory;

nst::u64  _reported_memory_size() {
	return 0;
}
nst::u64 calc_total_use() {
	return 0;
}
/// accessors for journal stats
void set_store_journal_size(nst::u64 ns) {
	store_journal_size = ns;
}
nst::u64 get_store_journal_lower_max() {
	return store_journal_lower_max;
}
void add_btree_totl_used(ptrdiff_t added) {

}
void remove_btree_totl_used(ptrdiff_t added) {

}

bool register_connector() {

	Poco::Data::SQLite::Connector::registerConnector();

	return true;
}
static bool registered = register_connector();
