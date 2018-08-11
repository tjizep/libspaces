// spaces.cpp : Defines the exported functions for the DLL application.
//
#ifdef __cplusplus
extern "C" {
#include <lua.h>
}
#endif

#include <storage/spaces/data_type.h>

#include <storage/interface/space_lua.h>

#include <storage/network/replication.h>

#include <storage/transactions/abstracted_storage.h>
extern void start_storage();
extern void stop_storage();

DEFINE_SESSION_KEY(SPACES_SESSION_KEY);
typedef spaces::session_t session_t;
typedef spaces::spaces_iterator<session_t::_Set> lua_iterator_t;
typedef rabbit::unordered_map<spaces::key, spaces::record> _KeyCache;
static std::string& lua_tostdstring(lua_State *L, int index){
	thread_local std::string result;
	result.clear();

	size_t l = 0;
	if(lua_isstring(L,index)){
		const char * s = luaL_checklstring(L,index,&l);

		if(s!=NULL){
			result.append(s,l);
		}
	}

	return result;
}

namespace spaces{
	session_t::_LuaKeyMap keys;

	space * is_space(lua_State *L, int at = 1) {
		if (lua_istable(L, at)) {
			ptrdiff_t pt = reinterpret_cast<ptrdiff_t>(lua_topointer(L, at));
			auto f = keys.find(pt);
			if(f!=keys.end()){
				return f->second;
			}
		}
		return nullptr;
	}
	space * get_space(lua_State *L, int at = 1) {
		space * space = is_space(L,at);
		if(space==nullptr){
			luaL_error(L, "no key associated with table of type %s", SPACES_LUA_TYPE_NAME);
		}
		return space;
	}
	void close(ptrdiff_t pt) {
		dbg_print("Closing spaces key... ");
		auto f = keys.find(pt);
		if (f!=keys.end()) {
			delete f->second;
			keys.erase(f);
			dbg_print("ok ");
		}else{
			dbg_print("not found");
		}
	}
	session_t::lua_iterator*  get_iterator(lua_State *L, int at = 1) {
		auto * i = err_checkudata<session_t::lua_iterator>(L, SPACES_ITERATOR_LUA_TYPE_NAME, at);

		if(i==nullptr || !i->has_session()){
			luaL_error(L, "invalid connection or session associated with iterator %s", SPACES_LUA_TYPE_NAME);
		}
		if(!i->get_session()->reader()){
			i->recover();
		}

		return i;

	}


}
static int  l_space_close(lua_State *L) {
	stop_storage();
	return 0;
}
static int l_serve_space(lua_State *L) {
    nst::u32 port = spaces::DEFAULT_PORT;
    if(lua_isnumber(L,1)){
        port = lua_tointeger(L,1);
    }
    spaces::block_replication_server server(port);
	server.run();

    return  0;
}
static int l_seed_space(lua_State *L) {
    try{
        if (lua_isstring(L, 1) && lua_isnumber(L, 2)) {
			const char * ip = lua_tostring(L,1);
			nst::u16 port = lua_tointeger(L,2);
			stored::get_abstracted_storage(STORAGE_NAME)->add_seed(ip,port);


		}
    }catch(std::exception& e){
        luaL_error(L,"could not connect: %s",e.what());
    }
    return 0;
}
static int l_space_local_writes(lua_State *L) {
    try{

        stored::get_abstracted_storage(STORAGE_NAME)->set_local_writes(lua_toboolean(L,1));
    }catch(std::exception& e){
        luaL_error(L,"could not set local writes: %s",e.what());
    }
	return 0;
}
static int l_space_observe(lua_State* L){
	try{

		if (lua_isstring(L, 1) && lua_isnumber(L, 2)) {
			const char *ip = lua_tostring(L, 1);
			nst::u16 port = lua_tointeger(L, 2);

			stored::get_abstracted_storage(STORAGE_NAME)->get_replication_control()->set_observer(ip,port);
		}
	}catch(std::exception& e){
		luaL_error(L,"could not set observer: %s",e.what());
	}
	return 0;
}
static int l_replicate_space(lua_State *L) {
    try{
        if (lua_isstring(L, 1) && lua_isnumber(L, 2)) {
            const char * ip = lua_tostring(L,1);
			nst::u16 port = lua_tointeger(L,2);
			stored::get_abstracted_storage(STORAGE_NAME)->add_replicant(ip,port);

        }
    }catch(std::exception& e){
        luaL_error(L,"could not connect: %s",e.what());
    }
    return 0;
}
static int l_open_session(lua_State *L) {
	const char * name = STORAGE_NAME;
	if (lua_isstring(L, 1)) {
		name = lua_tostring(L,1);
	}
	auto s = spaces::create_session<session_t>(L,name, false);
	s->set_mode(true); /// start a reading transaction
	s->begin(); /// will start transaction
	return 1;
}
static int l_session_open_space(lua_State *L) {
	auto s = spaces::get_session<spaces::session_t>(L);
	s->set_mode(true); /// if the transaction is already started this wont make a difference
    s->begin(); /// will start transaction
	session_t::space*  r = s->open_space(spaces::keys,s,0);

	if (s->get_set().size() != 0) {
		s->resolve_id(r);
	}
	return 1;
}
static int l_session_read(lua_State *L) {
	spaces::get_session<session_t>(L);

	return 0;
}
static int l_session_write(lua_State *L) {
	spaces::get_session<session_t>(L);

	return 0;;
}

static int l_session_from(lua_State *L) {
	return 0;
}

static int l_session_begin(lua_State *L) {

	auto s = spaces::get_session<session_t>(L);

	s->begin();
	return 0;
}
static int l_session_begin_reader(lua_State *L) {
	auto s = spaces::get_session<session_t>(L);
	s->set_mode(true);
	s->begin();
	return 0;
}

static int l_session_commit(lua_State *L) {

	spaces::get_session<session_t>(L)->commit();
	return 0;
}

static int l_session_rollback(lua_State *L) {

	spaces::get_session<session_t>(L)->rollback();
	return 0;
}

static int l_space_debug(lua_State* L) {
	nst::storage_debugging = true;
	nst::storage_info = true;

	return 0;
}
static int l_space_quiet(lua_State* L) {
	nst::storage_debugging = false;
	nst::storage_info = false;
	return 0;
}
static int l_configure_space(lua_State *L) {
	if (lua_isstring(L, 1)) {
		nst::data_directory = lua_tostring(L,1);
		dbg_print("storage directoryt set to %s",nst::data_directory.c_str());
	}
	return 0;
}
static int l_setmaxmb_space(lua_State *L) {
	nst::u64 mmb = (nst::u64)lua_tonumber(L,1);
	dbg_print("set max mem to %lld",(nst::lld)mmb);
	allocation_pool.set_max_pool_size((1024UL*1024UL*mmb)/2UL);
	//buffer_allocation_pool.set_max_pool_size((1024UL*1024UL*mmb*1UL)/4UL);
    return 0;
}


static const struct luaL_Reg spaces_f[] = {
	{ "open", l_open_session },
	{ "storage", l_configure_space },
    { "serve", l_serve_space},
    { "setMaxMb", l_setmaxmb_space },
    { "replicate", l_replicate_space },
    { "seed", l_seed_space },
	{ "debug", l_space_debug },
	{ "quiet", l_space_quiet },
    { "localWrites", l_space_local_writes },
	{ "observe", l_space_observe },
	{ "__gc", l_space_close },
	{ NULL, NULL } /* sentinel */
};


static int spaces_close(lua_State *L) {
	spaces::space * p = spaces::get_space(L);
	ptrdiff_t pt = reinterpret_cast<ptrdiff_t>(lua_topointer(L, 1));
	spaces::close(pt);
	return 0;
}
static int spaces_len(lua_State *L) {
	int t = lua_gettop(L);

	spaces::space * p = spaces::get_space(L);
	lua_pushnumber(L, p->session->len(p));
	t = lua_gettop(L);
	return 1;
}

static int spaces_less_equal(lua_State *L) {

	spaces::space * l = spaces::get_space(L,1);
	spaces::space * r = spaces::get_space(L,2);
	lua_pushboolean(L, !(r->first < l->first));
	return 1;

}
static int spaces_less_than(lua_State *L) {

	spaces::space * l = spaces::get_space(L,1);
	spaces::space * r = spaces::get_space(L,2);
	lua_pushboolean(L, l->first < r->first);
	return 1;
}
static int spaces_equal(lua_State *L) {

	spaces::space * l = spaces::get_space(L,1);
	spaces::space * r = spaces::get_space(L,2);
	lua_pushboolean(L, !(r->first != l->first));

	return 1;
}

static int spaces_newindex(lua_State *L) {
	// will be t,k,v <-> 1,2,3
	int t = lua_gettop(L);
	if(t!=3) luaL_error(L,"invalid argument count for subscript assignment");
	spaces::space k;
	spaces::space* p = spaces::get_space(L,1);
	auto s = std::static_pointer_cast<session_t>(p->session);

	s->begin_writer();
	s->resolve_id(p);
    k.first.set_context(p->second.get_identity());
	s->to_space_data( k.first.get_name(), 2);


	if(lua_isnil(L,3)){
		s->erase(k.first);
	}else{
		s->to_space(spaces::keys,k, 3);
		s->insert_or_replace(k);
	}


	
	return 0;
}
/**
 * resolve a route if one is flagged
 * @param s the session which will be used
 * @param value the value of the context,key,value tuple
 * @return the original session if no route was specified
 */
static session_t::ptr resolve_route(const session_t::ptr &s,const spaces::record& value){
	if(value.is_flag(spaces::record::FLAG_ROUTE)){ /// this is a route
		const char * storage = s->map_data(value).get_value().get_sequence().c_str();
		if(s->get_name() != storage ){
			/// create a new session for the storage ??? no sessions are cached per initial session
			auto r = s->create_session(storage, s);
			r->set_mode(true); /// start a read transaction
			r->begin(); /// will start transaction
			return r;
		}
	}
	return s;
}
/**
 * push a space or data onto the lua stack
 * if the space has an identity a space object is pushed
 * else one of the lua primitive types are pushed
 * @param L the lua state
 * @param ps the session
 * @param key the key of potential
 * @param value
 * @return 1 if something was pushed 0 otherwize
 */
static int push_space(lua_State*L, const session_t::ptr &ps,const spaces::key& key, const spaces::record& value){
	if (!value.is_flag(spaces::record::FLAG_LARGE) && value.get_identity() != 0) {
		auto s = resolve_route(ps,value);
		spaces::space *r = s->open_space(spaces::keys,s,value.get_identity());
		ptrdiff_t pt = reinterpret_cast<ptrdiff_t>(lua_topointer(L, -1));
		spaces::keys[pt] = r;

		r->first = key;
		r->second = value;

	} else {
		ps->push_data(ps->map_data(value).get_value());
	}
	return 1;
}
/**
 * implements __index meta method
 * if the object could not be found nil is pushed
 * @param L the lua state
 * @return 1
 */
static int spaces_index(lua_State *L) {
	spaces::space* p = spaces::get_space(L,1);
	auto s = std::static_pointer_cast<session_t>(p->session);
	s->begin(); /// use whatever mode is set
	spaces::space k;

	if (p->second.get_identity() != 0) {

		s->to_space_data(k.first.get_name(), 2);
		k.first.set_context(p->second.get_identity());

		//auto i = s->get_set().find(k.first); /// a non hash optimized lookup
		auto value = s->get_set().direct(k.first);/// a hash optimized lookup

		if (value != nullptr) {
			return push_space(L,s,k.first,*value);
		} else {
			lua_pushnil(L);
		}



	}
	else { /// cannot index something thats not a parent
		lua_pushnil(L);
	}

	return 1;

}
/**
 * pushes a number optionally converting a space object to a number
 * @param L the lua state
 * @param at where on the lua state the conversion will take place
 * @return 1
 */
static f8 to_number(lua_State *L, i4 at) {
	f8 r = 0.0;
	if (lua_isnumber(L, at)) {
		r = lua_tonumber(L, at);
	}
	else {

		spaces::space* p = spaces::get_space(L,at);
		if (p) {
			r = p->second.get_value().to_number();
		}
	}
	return r;
}
static int spaces_add(lua_State *L) {
	lua_pushnumber(L, to_number(L, 1) + to_number(L, 2));
	return 1;
}
static int spaces_sub(lua_State *L) {
	lua_pushnumber(L, to_number(L, 1) - to_number(L, 2));
	return 1;
}
static int spaces_neg(lua_State* L) {
	lua_pushnumber(L, -to_number(L, 1));
	return 1;
}
static int spaces_mul(lua_State* L) {
	lua_pushnumber(L, to_number(L, 1)*to_number(L, 2));
	return 1;
}
static int spaces_pow(lua_State* L) {
	lua_pushnumber(L, pow(to_number(L, 1), to_number(L, 2)));
	return 1;
}
static int spaces_div(lua_State* L) {
	lua_pushnumber(L, to_number(L, 1) / to_number(L, 2));
	return 1;
}
static int spaces_tostring(lua_State *L) {
	spaces::space* p = spaces::get_space(L,1);
	auto s = std::static_pointer_cast<session_t>(p->session);
	if (p->second.get_identity() != 0) {
		lua_pushfstring(L, "table: %p", p->second.get_identity());
		return 1;
	}
	umi l = 0;
	std::string value;
	if (p != nullptr) {
		s->push_data(p->second.get_value());
		return 1;
	}
	return 0;
}
static int spaces_metatable(lua_State *L) {
	lua_newtable(L);
	return 1;
}
static int spaces_type(lua_State* L) {
	lua_pushstring(L, "table");
	return 1;
}
static int l_pairs_iter(lua_State* L) { //i,k,v
	auto *i = spaces::get_iterator(L,lua_upvalueindex(1));
	if (!i->end()) {

		int r = i->get_session()->push_pair(spaces::keys,i->get_session(),spaces::get_key(i->get_i()),spaces::get_data(i->get_i()));
		i->next();
		return r;
	}
	
	return 0;
}
/**
 * set an iterator meta table to a negative position in stack
 * @param L lua state
 * @param at negative position relative to top
 */
static void set_iter_meta(lua_State* L,int at){
    if(at >= 0){
        luaL_error(L, "invalid parameter");
    }
    // set its metatable
    luaL_getmetatable(L, SPACES_ITERATOR_LUA_TYPE_NAME);
    if (lua_isnil(L, -1)) {
        luaL_error(L, "no meta table of type %s", SPACES_ITERATOR_LUA_TYPE_NAME);
    }
    if(at < 0)
        lua_setmetatable(L, at-1);
}
static int push_iterator(lua_State* L, session_t::ptr s, spaces::space* p, const spaces::data& lower, const spaces::data& upper){


	spaces::key f,e ;

	f.set_context(p->second.get_identity());
	e.set_context(p->second.get_identity());
	f.set_name(lower);
	e.set_name(upper);

	// create the lua iterator
	auto * pi = s->create_iterator(s);
	pi->set_lower(s->get_set(),f);
	pi->set_upper(s->get_set(),e);

	// set its metatable
    set_iter_meta(L,-1);


	return 1;
}
static int push_iterator_closure(lua_State* L, session_t::ptr s, spaces::space* p, const spaces::data& lower, const spaces::data& upper){


	push_iterator(L,s,p,lower,upper);

	lua_pushcclosure(L, l_pairs_iter, 1);//i

	return 1;
}
inline const spaces::data make_inf(){
	spaces::data inf;
	inf.make_infinity();
	return inf;
}
static int spaces___pairs(lua_State* L) {

	spaces::space* p = spaces::get_space(L,1);
	auto s = std::static_pointer_cast<session_t>(p->session);
	s->begin(); /// will start a transaction
	return push_iterator_closure(L, s, p, spaces::data(),make_inf());
}

static int spaces_call(lua_State *L) {
	int t = lua_gettop(L);

	int o = (t >= 4) ? 1: 0;
	spaces::space* p = spaces::get_space(L,1);
    auto s = std::static_pointer_cast<session_t>(p->session);
    s = resolve_route(s, p->second);
    s->begin(); /// will start a transaction
	spaces::data lower;
	spaces::data upper = make_inf();

	if(t == 2){
		return push_iterator(L, s, p, lower, upper);
    }


	if(t >= o + 2)
		s->to_space_data(lower, o + 2);
	if(t >= o + 3)
		s->to_space_data(upper, o + 3);
	return push_iterator_closure(L, s, p, lower, upper);


}

// meta table for spaces
const struct luaL_Reg spaces_m[] = {
	{ "__index", spaces_index },
	//{ "__intex", spaces_intex },
	{ "__newindex", spaces_newindex },
	//{ "__tostring", spaces_tostring },
	{ "__metatable", spaces_metatable },
	{ "__call", spaces_call },
	//{ "__type", spaces_type },
	//{ "__add",spaces_add },
	//{ "__sub",spaces_sub },
	//{ "__mul",spaces_mul },
	//{ "__div",spaces_div },
	//{ "__unm",spaces_neg },
	//{ "__pow",spaces_pow },
	{ "__gc", spaces_close },
	{ "__len", spaces_len },
	//{ "__le", spaces_less_equal },
	//{ "__lt", spaces_less_than },
	//{ "__eq", spaces_equal },
	{ "__pairs", spaces___pairs },
	{ NULL, NULL }/* sentinel */
};

static int l_pairs_iter_start(lua_State* L){
	auto *i = spaces::get_iterator(L,1);
	i->start();
    return 0;
}
static int l_pairs_iter_last(lua_State* L){
	auto *i = spaces::get_iterator(L,1);
	i->last();
    return 0;
}
static int l_pairs_iter_count(lua_State* L) {
	auto *i = spaces::get_iterator(L,1);

	lua_pushinteger(L,i->count());

	return 1;

}
static int l_pairs_iter_key(lua_State* L){

    auto *i = spaces::get_iterator(L,1);
    if(lua_gettop(L) > 1){
        long long index = lua_tonumber(L,2);
        /// use the lua index convention
        return i->get_session()->push_data(spaces::get_key(i->get_at(index-1)).get_name());
    }

	if (!i->end())
		return i->get_session()->push_data(spaces::get_key(i->get_i()).get_name());
	lua_pushnil(L);
	return 1;

}
static int l_pairs_iter_first_key(lua_State* L){

	auto *i = spaces::get_iterator(L,1);
	return i->get_session()->push_data(spaces::get_key(i->get_first()).get_name());

}
static int l_pairs_iter_last_key(lua_State* L){

	auto *i = spaces::get_iterator(L,1);
	return i->get_session()->push_data(spaces::get_key(i->get_first()).get_name());

}
static int l_pairs_iter_value(lua_State* L){
	auto *i = spaces::get_iterator(L,1);
	if(lua_gettop(L) > 1){
		long long index = lua_tonumber(L,2);
		/// use the lua index convention
		auto iter = i->get_at(index-1);
		return push_space(L, i->get_session(), spaces::get_key(iter), spaces::get_data(iter));
	}
	if (!i->end())
		return push_space(L, i->get_session(), spaces::get_key(i->get_i()), spaces::get_data(i->get_i()));

	lua_pushnil(L);
	return 1;

}
static int l_pairs_iter_pair(lua_State* L){
    auto *i = spaces::get_iterator(L,1);
    if (!i->end()) {
        return i->get_session()->push_pair(spaces::keys, i->get_session(), spaces::get_key(i->get_i()), spaces::get_data(i->get_i()));
    }else{
        lua_pushnil(L);
        lua_pushnil(L);
    }
    return 2;
}
static int l_pairs_iter_next(lua_State* L){
	auto *i = spaces::get_iterator(L,1);
    if (!i->end()) {
        i->next();
    }
    return 0;
}
static int l_pairs_iter_move(lua_State* L){
	if(lua_gettop(L) != 2) luaL_error(L,"invalid parameter to move");
	auto *i = spaces::get_iterator(L,1);
	long long index = lua_tonumber(L,2);
	i->move(index);
	return 0;
}
static int l_pairs_iter_valid(lua_State* L){
	auto *i = spaces::get_iterator(L,1);
	lua_pushboolean(L,!i->end());
	return 1;
}
static int l_pairs_iter_empty(lua_State* L){
	auto *i = spaces::get_iterator(L,1);
	lua_pushboolean(L,i->empty());
	return 1;
}
static int l_pairs_iter_prev(lua_State* L){
	auto *i = spaces::get_iterator(L,1);
	if(!i->first()){
		i->previous();
	}
    return 0;
}
static int l_pairs_iter_first(lua_State* L){
	auto *i = spaces::get_iterator(L,1);
	lua_pushboolean(L,i->first());
	return 1;
}
static int l_pairs_iter_first_value(lua_State* L){
	auto *i = spaces::get_iterator(L,1);
	auto iter = i->get_first();
	return push_space(L, i->get_session(), spaces::get_key(iter), spaces::get_data(iter));

}
static int l_pairs_iter_last_value(lua_State* L){
	auto *i = spaces::get_iterator(L,1);
	auto iter = i->get_last();
	return push_space(L, i->get_session(), spaces::get_key(iter), spaces::get_data(iter));
first
}
static int l_pairs_iter_close(lua_State* L){
	auto *i = spaces::get_iterator(L,1);
	dbg_print("destroy iterator");
	return 0;
}
static const struct luaL_Reg spaces_iter_f[] = {


	{ NULL, NULL } /* sentinel */
};

static const struct luaL_Reg spaces_iter_m[] = {
	{ "first",l_pairs_iter_first },
	{ "firstValue",l_pairs_iter_first_value },
	{ "start",l_pairs_iter_start },
	{ "last",l_pairs_iter_last },
	{ "lastValue",l_pairs_iter_last_value },
	{ "key",l_pairs_iter_key },
	{ "firstKey",l_pairs_iter_first_key },
	{ "lastKey",l_pairs_iter_last_key },
	{ "count",l_pairs_iter_count },
	{ "value",l_pairs_iter_value },
	{ "pair",l_pairs_iter_pair },
	{ "next",l_pairs_iter_next },
	{ "previous",l_pairs_iter_prev },
	{ "valid",l_pairs_iter_valid },
	{ "move",l_pairs_iter_move },
	{ "empty",l_pairs_iter_empty },
    { "__gc",l_pairs_iter_close },
    { NULL, NULL }/* sentinel */
};

static int l_session_close(lua_State* L) { 	
	auto s = spaces::get_session<session_t>(L, SPACES_SESSION_KEY);
	s.~shared_ptr<session_t>();
	if (spaces::db_session_count == 0) {
		stop_storage();
	}
	return 0;
}
static const struct luaL_Reg spaces_session_f[] = {

	{ NULL, NULL } /* sentinel */
};

static const struct luaL_Reg spaces_session_m[] =
{
	{ "open", l_session_open_space },
	{ "read", l_session_read },
	{ "write", l_session_write },
	{ "from", l_session_from },
	{ "begin", l_session_begin },
	{ "beginRead", l_session_begin_reader },
	{ "commit", l_session_commit },
	{ "rollback", l_session_rollback },
	{ "__gc",l_session_close },	
	{ NULL, NULL }/* sentinel */
};

#if 0
#endif


extern "C" int
#ifdef _MSC_VER
__declspec(dllexport)
#endif
luaopen_spaces(lua_State * L) {
	dbg_print("open lua spaces");
	start_storage();
	spaces::luaopen_plib_any(L, spaces_m, SPACES_LUA_TYPE_NAME, spaces_f, SPACES_NAME);
	spaces::luaopen_plib_any(L, spaces_iter_m, SPACES_ITERATOR_LUA_TYPE_NAME, spaces_iter_f, SPACES_ITERATOR_NAME);
	lua_pop(L,1);
	spaces::luaopen_plib_any(L, spaces_session_m, SPACES_SESSION_LUA_TYPE_NAME, spaces_session_f, SPACES_SESSION_NAME);
	//spaces::luaopen_plib_any(L, spaces_recursor_m, SPACES_LUA_RECUR_NAME, spaces_recursor_f, "_spaces_recursor");
    lua_pop(L,1);
	return 1;
}
extern "C" int
#ifdef _MSC_VER
__declspec(dllexport)
#endif
luaclose_spaces(lua_State * L) {
	stop_storage();
	return 0;
}