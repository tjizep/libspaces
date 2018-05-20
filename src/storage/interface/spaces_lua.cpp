// spaces.cpp : Defines the exported functions for the DLL application.
//
#ifdef __cplusplus
extern "C" {
#include <lua.h>
}
#endif
#include <helper/lua_xt.h>

#include <storage/spaces/data_type.h>

#include <storage/interface/space_lua.h>

#include <storage/network/replication.h>

#include <storage/transactions/abstracted_storage.h>

DEFINE_SESSION_KEY(SPACES_SESSION_KEY);
typedef spaces::lua_session<spaces::db_session> session_t;
//typedef spaces::lua_session<spaces::mem_session> session_t;
typedef spaces::spaces_iterator<session_t::_Set> lua_iterator_t;
typedef rabbit::unordered_map<spaces::key, spaces::record> _KeyCache;

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

static int l_open_space(lua_State *L) {
	if (lua_isstring(L, 1)) {

	}
	session_t * s = spaces::create_session<session_t>(L, SPACES_SESSION_KEY,false);
	spaces::space*  r = s->open_space(0);
	if (s->get_set().size() != 0) {
		s->resolve_id(r);
	}
	return 1;
}
static int l_read(lua_State *L) {
	if (lua_isstring(L, 1)) {

	}
	spaces::create_session<session_t>(L, SPACES_SESSION_KEY,true);

	return 0;
}
static int l_write(lua_State *L) {
	if (lua_isstring(L, 1)) {

	}
	spaces::create_session<session_t>(L, SPACES_SESSION_KEY,false);

	return 0;
}

static int l_from_space(lua_State *L) {
	return 1;
}

static int l_begin_space(lua_State *L) {
	spaces::create_session<session_t>(L, SPACES_SESSION_KEY,false)->begin();
	return 0;
}
static int l_begin_reader_space(lua_State *L) {
	spaces::create_session<session_t>(L, SPACES_SESSION_KEY,true)->begin();
	return 0;
}

static int l_commit_space(lua_State *L) {
	spaces::get_session<session_t>(L, SPACES_SESSION_KEY)->commit();
	return 0;
}

static int l_rollback_space(lua_State *L) {

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
	}
	return 0;
}
static int l_setmaxmb_space(lua_State *L) {
	nst::u64 mmb = (nst::u64)lua_tonumber(L,1);
	allocation_pool.set_max_pool_size((1024UL*1024UL*mmb*3UL)/4UL);
	buffer_allocation_pool.set_max_pool_size((1024UL*1024UL*mmb*1UL)/4UL);
    return 0;
}


static const struct luaL_Reg spaces_f[] = {
	{ "storage", l_configure_space },
    { "serve", l_serve_space},
	{ "open", l_open_space },
	{ "read", l_read },
	{ "write", l_write },
	{ "from", l_from_space },
	{ "begin", l_begin_space },
	{ "beginRead", l_begin_reader_space },

	{ "commit", l_commit_space },
	{ "rollback", l_rollback_space },
    { "setMaxMb", l_setmaxmb_space },
    { "replicate", l_replicate_space },
    { "seed", l_seed_space },
	{ "debug", l_space_debug },
	{ "quiet", l_space_quiet },
    { "localWrites", l_space_local_writes },
	{ "observe", l_space_observe },
	{ NULL, NULL } /* sentinel */
};

static int spaces_close(lua_State *L) {
	
	ptrdiff_t pt = reinterpret_cast<ptrdiff_t>(lua_topointer(L, 1));
	spaces::get_session<session_t>(L, SPACES_SESSION_KEY)->close(pt);	
	return 0;
}
static int spaces_len(lua_State *L) {
	int t = lua_gettop(L);
	auto s = spaces::get_session<session_t>(L, SPACES_SESSION_KEY);
	spaces::space * p = s->get_space();
	lua_pushnumber(L, (lua_Number)s->len(p));
	t = lua_gettop(L);
	return 1;
}

static int spaces_less_equal(lua_State *L) {
	auto s = spaces::get_session<session_t>(L, SPACES_SESSION_KEY);
	spaces::space * l = s->get_space(1);
	spaces::space * r = s->get_space(2);
	lua_pushboolean(L, !(r->first < l->first));
	return 1;

}
static int spaces_less_than(lua_State *L) {
	auto s = spaces::get_session<session_t>(L, SPACES_SESSION_KEY);
	spaces::space * l = s->get_space(1);
	spaces::space * r = s->get_space(2);
	lua_pushboolean(L, l->first < r->first);
	return 1;
}
static int spaces_equal(lua_State *L) {
	auto s = spaces::get_session<session_t>(L, SPACES_SESSION_KEY);
	spaces::space * l = s->get_space(1);
	spaces::space * r = s->get_space(2);
	lua_pushboolean(L, !(r->first != l->first));

	return 1;
}

static int spaces_newindex(lua_State *L) {
	// will be t,k,v <-> 1,2,3
	auto s = spaces::get_session<session_t>(L, SPACES_SESSION_KEY);
    s->set_mode(false); /// must write
    s->begin(); /// will start the writing transaction
	spaces::space k;
	spaces::space* p = s->get_space(1);

	s->resolve_id(p);
    k.first.set_context(p->second.get_identity());
	s->to_space_data( k.first.get_name(), 2);
	s->to_space(k, 3);


	if(!lua_isnil(L,3))
		s->insert_or_replace(k);

	
	return 0;
}



static int spaces_index(lua_State *L) {
	auto s = spaces::get_session<session_t>(L, SPACES_SESSION_KEY);

	s->begin(); /// use whatever mode is set
	spaces::space k;
	spaces::space *r = nullptr;
	spaces::space* p = s->get_space();

	if (p->second.get_identity() != 0) {

		s->to_space_data(k.first.get_name(), 2);
		k.first.set_context(p->second.get_identity());

		//auto i = s->get_set().find(k.first);
		auto value = s->get_set().direct(k.first);

		if (value != nullptr) {

			if (!value->is_flag(spaces::record::FLAG_LARGE) && value->get_identity() != 0) {
				r = s->open_space(value->get_identity());
				r->first = k.first;
				r->second = *value;

			} else {
				s->push_data(s->map_data(*value).get_value());
			}

		} else {
			lua_pushnil(L);
		}



	}
	else { /// cannot index something thats not a parent
		lua_pushnil(L);
	}

	
	return 1;

}
static f8 to_number(lua_State *L, i4 at) {
	f8 r = 0.0;
	if (lua_isnumber(L, at)) {
		r = lua_tonumber(L, at);
	}
	else {
		auto s = spaces::get_session<session_t>(L, SPACES_SESSION_KEY);
		spaces::space* p = s->get_space(at);
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
	auto s = spaces::get_session<session_t>(L, SPACES_SESSION_KEY);
	spaces::space* p = s->get_space(1);
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
#if 0



static int spaces_call_environment(lua_State *L) {
	try {
		space_service * pspace = get_space(L, lua_upvalueindex(1));
		umi depth = 0;
		if (lua_isstring(L, 2)) {
			const char * cn = lua_tostring(L, 2);
			spaces_key k;
			to_space_key(L, pspace, k, ipKey(), interpretation::flat, 2);

			for (resolution_iterator ri(k, pspace, pspace->get_path_context()); ri != ri.end(); ++ri) {
				const spaces_key &lk = (*ri);
				if (lk.get_meta_char() == string_types::table) {
					ri.push(L);
					return 1;
				}
				else if (lk.get_meta_char() == string_types::function) {
					if (ri.get_depth()>0) {//neccessary eeviel
						ri.push(L);
						return 1;
					}
				}
				else {auto s = spaces::get_session<session_t>(L, SPACES_SESSION_KEY);
	s->begin(); /// will start a transaction
	spaces::space* p = s->get_space();// its at stack 1 because the function is called
	spaces::key f,e ;
	f.set_context(p->second.get_identity());
	e.set_context(p->second.get_identity());
	e.get_name().make_infinity();

	// create the lua iterator
	lua_iterator_t * pi = s->create_iterator();
	pi->i = s->get_set().lower_bound(f);
	pi->e = s->get_set().lower_bound(e);

	// set its metatable
	luaL_getmetatable(L, SPACES_ITERATOR_LUA_TYPE_NAME);
	if (lua_isnil(L, -1)) {
		luaL_error(L, "no meta table of type %s", SPACES_ITERATOR_LUA_TYPE_NAME);
	}
	lua_setmetatable(L, -2);

	lua_pushcclosure(L, l_pairs_iter, 1);//i
	///
	return 1; // its adding something to the stack
					return push_key(L, lk, ipValue());
				}
			}
		}
		// ok now check _G	
		i4 t = lua_gettop(L);
		lua_gettable(L, LUA_GLOBALSINDEX);
		t = lua_gettop(L);
	}
	catch (allexceptions&all) {
		get_session_space(L)->reset();
		luaL_error(L, "spaces:%s", all.getdescription().c_str());
	}
	return 1;
}

static int spaces_call(lua_State *L) {
	try {
		space_service * pspace = get_space(L);
		i4 r = 0;
		if (is_space(L, 2)) { //this might be an api call
			string name;
			pspace->get_current().get_name(name);
			if (!pspace->is_api(name)) {

				switch (pspace->get_anapi()) {
				case apis::clone:
					//TODO: pspace->path->context replaced with NULL
					make_space(L, pspace->get_current(), 0);
					return 1;
				case apis::context:
					//TODO: this wont work if paths are switched of
					if (pspace->has_path_context()) {
						make_space(L, pspace->get_path_context()->get_key(), pspace->get_path_context()->context);
					}
					else
						lua_pushnil(L);
					return 1;
				case apis::find:
					make_space_iter(L, pspace)->begin();
					return 1;
				case apis::lower: {
					spaces_key bound;
					to_space_key(L, pspace, bound, ipKey(), interpretation::flat, 3);//only interested in the values not the keys
					make_space_iter(L, pspace)->lower(bound);
				}
								  break;
				case apis::name: {
					string r;
					pspace->get_current().get_name(r);
					lua_pushlstring(L, &r[0], r.size());
					return 1;
				}
								 break;
				default:
					break;
				};

			};//else let it fall through as overridden
		}
		if (pspace->get_current().get_meta_char() == string_types::function) {
			i4 passed = (i4)lua_gettop(L);
			const spaces_uuid_t& identity = pspace->get_current().get_identity();
			lua_pushlstring(L, identity.p(), identity.size());
			lua_gettable(L, LUA_GLOBALSINDEX);
			if (lua_isnil(L, -1)) {
				lua_pop(L, 1);
				lua_pushlstring(L, identity.p(), identity.size());
				_DumpVector buff;
				pspace->get_current().to_string<_DumpVector>(buff);
				r = luaL_loadbuffer(L, (const i1*)&buff[0], buff.size(), "funct");
				if (r != 0) {
					luaL_error(L, "this space does not contain a function");
				}
				i4 funct = lua_gettop(L);
				lua_newtable(L);// the environment table
				i4 env = lua_gettop(L);
				lua_newtable(L);// the environments meta table
				i4 meta = lua_gettop(L);
				lua_pushvalue(L, 1);// the upvalue for the closure
				lua_pushcclosure(L, spaces_call_environment, 1);
				lua_setfield(L, meta, "__index"); // leaves the intended meta table	i.e. meta[__index] = function
				lua_setmetatable(L, env); // only the environment table should be left after this
				lua_setfenv(L, funct); // only t6he function is left
				lua_settable(L, LUA_GLOBALSINDEX);
				lua_pushlstring(L, identity.p(), identity.size());
				lua_gettable(L, LUA_GLOBALSINDEX);
			}
			for (i4 a = 2; a <= passed; ++a) {
				lua_pushvalue(L, a);
			}
			lua_call(L, passed - 1, LUA_MULTRET);
			r = lua_gettop(L) - passed;
			return r;
		}
	}
	catch (allexceptions&all) {
		get_session_space(L)->reset();
		luaL_error(L, "spaces:%s", all.getdescription().c_str());
	}
	return 0;
}s
#endif
static int l_pairs_iter(lua_State* L) { //i,k,v 
	auto s = spaces::get_session<session_t>(L, SPACES_SESSION_KEY);

	lua_iterator_t *i = s->get_iterator(lua_upvalueindex(1));
	if (!i->end()) {

		s->push_pair(spaces::get_key(i->i),spaces::get_data(i->i));
		i->next();
		return 2;
	}
	
	return 0;
}
static int push_iterator(lua_State* L, session_t* s, spaces::space* p, const spaces::data& lower, const spaces::data& upper){


	spaces::key f,e ;

	f.set_context(p->second.get_identity());
	e.set_context(p->second.get_identity());
	f.set_name(lower);
	e.set_name(upper);

	// create the lua iterator
	lua_iterator_t * pi = s->create_iterator();
	pi->i = s->get_set().lower_bound(f);
	pi->e = s->get_set().lower_bound(e);

	// set its metatable
	luaL_getmetatable(L, SPACES_ITERATOR_LUA_TYPE_NAME);
	if (lua_isnil(L, -1)) {
		luaL_error(L, "no meta table of type %s", SPACES_ITERATOR_LUA_TYPE_NAME);
	}
	lua_setmetatable(L, -2);

	lua_pushcclosure(L, l_pairs_iter, 1);//i
	///
	return 1;
}
inline const spaces::data make_inf(){
	spaces::data inf;
	inf.make_infinity();
	return inf;
}
static int spaces___pairs(lua_State* L) {

	auto s = spaces::get_session<session_t>(L, SPACES_SESSION_KEY);
	s->begin(); /// will start a transaction
	spaces::space* p = s->get_space();// its at stack 1 because the function is called
	return push_iterator(L, s, p, spaces::data(),make_inf());
}
static std::string range = "range";
static int spaces_call(lua_State *L) {
	int t = lua_gettop(L);

	int o = (t >= 4) ? 1: 0;
	auto s = spaces::get_session<session_t>(L, SPACES_SESSION_KEY);
	s->begin(); /// will start a transaction
	spaces::space* p = s->get_space();// its at stack 1 because the function is called
	spaces::data lower;
	spaces::data upper = make_inf();
	if(t >= o + 2)
		s->to_space_data(lower, o + 2);
	if(t >= o + 3)
		s->to_space_data(upper, o + 3);
	return push_iterator(L, s, p, lower, upper);


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


static const struct luaL_Reg spaces_iter_f[] = {
	{ NULL, NULL } /* sentinel */
};

static const struct luaL_Reg spaces_iter_m[] =
{ 
	//{ "__gc",l_pairs_iter_close },	
{ NULL, NULL }/* sentinel */
};

static int l_session_close(lua_State* L) { 	
	auto s = spaces::get_session<session_t>(L, SPACES_SESSION_KEY);
	s->~session_t();
	return 0;
}
static const struct luaL_Reg spaces_session_f[] = {
	{ NULL, NULL } /* sentinel */
};

static const struct luaL_Reg spaces_session_m[] =
{
	{ "__gc",l_session_close },	
	{ NULL, NULL }/* sentinel */
};

#if 0
#endif


extern "C" int
#ifdef _MSC_VER_
__declspec(dllexport)
#endif
luaopen_spaces(lua_State * L) {
	// provides pairs and ipairs metamethods (__ added)
	/// lua_xt is required for 5.1 iterator compatibility
	//lua_XT::luaopen_xt(L);
	spaces::luaopen_plib_any(L, spaces_m, SPACES_LUA_TYPE_NAME, spaces_f, SPACES_NAME);
	spaces::luaopen_plib_any(L, spaces_iter_m, SPACES_ITERATOR_LUA_TYPE_NAME, spaces_iter_f, SPACES_ITERATOR_NAME);
	spaces::luaopen_plib_any(L, spaces_session_m, SPACES_SESSION_LUA_TYPE_NAME, spaces_session_f, SPACES_SESSION_NAME);
	//spaces::luaopen_plib_any(L, spaces_recursor_m, SPACES_LUA_RECUR_NAME, spaces_recursor_f, "_spaces_recursor");
	lua_newtable(L);
	lua_pushstring(L,"kv");
	lua_setfield(L,-2,"__mode");
	lua_setglobal(L, SPACES_G);
	return 1;
}
