#pragma once
extern "C" {
	#include <lualib.h>
	#include <lauxlib.h>
	#include <lua.h>	
};
#include "spaces_session.h"

INCLUDE_SESSION_KEY(SPACES_SESSION_KEY);
INCLUDE_SESSION_KEY(SPACES_MAP_ITEM);

#define SPACES_LUA_TYPE_NAME "spacesLT" //the spaces meta table name
#define SPACES_NAME "spaces" //the spaces meta table name
#define SPACES_ITERATOR_LUA_TYPE_NAME "spacesLIT" //the spaces iterator L(I)T
#define SPACES_ITERATOR_NAME "_spaces_iterators"
#define SPACES_SESSION_LUA_TYPE_NAME "spacesS" //the spaces session 
#define SPACES_SESSION_NAME "_spaces_sessions"
#define SPACES_WRAP_KEY "__$"

namespace spaces{

	static int luaopen_plib_any(lua_State *L, const luaL_Reg *m,
					 const char *object_name, 
					 const luaL_Reg *f,
					 const char * library_name)
	{
		luaL_newmetatable(L, object_name);
		lua_pushliteral(L, "__index");
		lua_pushvalue(L, -2);
		lua_rawset(L, -3);
		luaL_register (L, NULL, m);
		luaL_register(L, library_name, f);
		return 1;
	}
	
	
	static size_t lua_tostdstring(lua_State *L,std::string& ss, int at){
		size_t l = 0;
		const char * s  = lua_tolstring(L,at,&l);	
		ss.insert(ss.end(), s,s+l);
		return l;
	}
	static void * is_udata(lua_State *L, int ud, const char* ) {
		return lua_touserdata(L, ud);
		//return luaL_checkudata(L, ud, tname);
	}
	template<typename _TypeName>
	static _TypeName *err_checkudata(lua_State *L, const char* tname, int ud = 1) {
		void *p = is_udata(L, ud, tname);
		if (p != NULL) {  /* value is a userdata? */
			return (_TypeName*)p;
		}
 		luaL_typerror(L, ud, tname);  /* else error */
		return NULL;  /* to avoid warnings */
	}
	
	template<typename _sT>
	static _sT* create_instance_from_nothing(lua_State *L) {
		size_t nbytes = sizeof(_sT);
		_sT *k = (_sT *)lua_newuserdata(L, nbytes);
		new (k) _sT();
		return k;  /* new userdatum is already on the stack */

	}
	template<typename _sT, typename _pT>
	static _sT* create_instance_from_nothing(lua_State *L, _pT param) {
		size_t nbytes = sizeof(_sT);
		_sT *k = (_sT *)lua_newuserdata(L, nbytes);
		new (k) _sT(param);
		return k;  /* new userdatum is already on the stack */

	}
	template<typename _sT, typename _pT, typename _p1T >
	static _sT* create_instance_from_nothing(lua_State *L, _pT param, _p1T param1) {
		size_t nbytes = sizeof(_sT);
		_sT *k = (_sT *)lua_newuserdata(L, nbytes);
		new (k) _sT(param,param1);
		return k;  /* new userdatum is already on the stack */

	}
	template<typename _SessionType>
	static _SessionType* create_session(lua_State *L,const char* name, bool is_reader) {
		_SessionType* r = nullptr;

		r = create_instance_from_nothing<_SessionType,const char*,bool>(L,name,is_reader);
		r->set_state(L);
		// set its metatable
		luaL_getmetatable(L, SPACES_SESSION_LUA_TYPE_NAME);
		if (lua_isnil(L, -1)) {
			luaL_error(L, "no meta table of type %s", SPACES_SESSION_LUA_TYPE_NAME);
			return nullptr;
		}
		lua_setmetatable(L, -2);

		r->check();
		r->set_mode(is_reader);
		return r;
	}
	template<typename _SessionType>
	static _SessionType* get_session(lua_State *L, int at = 1) {
		_SessionType* r = nullptr;
		r = err_checkudata<_SessionType>(L, SPACES_SESSION_NAME, at);

		r->check();
		return r;
	}
	#define tofilep(L)	((FILE **)luaL_checkudata(L, 1, LUA_FILEHANDLE))



    template<typename _SessionType>
    class lua_session : public spaces_session<_SessionType>{
	public:
		typedef lua_session<_SessionType> session_type;
		typedef typename session_type::space space;
		typedef std::vector<i1> _DumpVector;
		typedef rabbit::unordered_map<ptrdiff_t, space*> _LuaKeyMap;
		/// despite improvements
		/// sometimes the type system still gets lost without
		/// a hint
		typedef session_type _Self;
    private:
		lua_State *L;

	public:

		typedef typename spaces_session<_SessionType>::_Set _Set;
		struct lua_iterator : public spaces::spaces_iterator<_Set>{
			lua_iterator() : session(nullptr){
			}
			lua_session * session;
		};
		lua_session(const char * name, bool reader) : L(nullptr),spaces_session<_SessionType>(name,reader) {
			dbg_print("create session:%s",name);
		}
		~lua_session() {
			dbg_print("closing session");

		}

		void set_state(lua_State *L) {
			this->L = L;
		}
        bool is_integer(double n) {
			return (n - (i8)n == 0);
		}
		lua_iterator* create_iterator() {
			auto i = create_instance_from_nothing<lua_iterator>(L); /* new userdatum is already on the stack */
			i->session = this;
			return i;
		}
		lua_iterator*  get_iterator(int at = 1) {
			lua_iterator * i = err_checkudata<lua_iterator>(L, SPACES_ITERATOR_LUA_TYPE_NAME, at);
			i->session = this;
			return i;

		}
		space* create_space_from_nothing() {
			return create_instance_from_nothing<space>(L);  /* new userdatum is already on the stack */

		}
		umi table_clustered_type(int at) {
			umi n = 0;
			if (!lua_istable(L, at)) {
				return 0;
			}
			lua_pushnil(L);
			i8 last = 0;
			while (lua_next(L, at) != 0) {
				// just set the key part from the value in lua
				umi tk = lua_type(L, -2);
				if (tk == LUA_TNUMBER) {
					double n = ::lua_tonumber(L, -2);
					if (is_integer(n) && n > 0) {
						last = (i8)n;
					}
					else {
						return 0;
					}
				}
				else {
					lua_pop(L, 2);
					return 0;
				}
				++n;
				lua_pop(L, 1);

			}

			return n;
		}

		template<typename _VectorType>
		struct vector_reader_state {
			vector_reader_state() :state(0) {};
			~vector_reader_state() {};
			i4 state;
			_VectorType v;
		};

		template<typename _VectorType>
		const char * vector_reader(void *ud, size_t *sz) {
			vector_reader_state<_VectorType> * state = (vector_reader_state<_VectorType>*)ud;

			if (state->state == 1 || state->v.empty()) {
				*sz = 0;
				return 0;
			}
			state->state = 1;
			*sz = state->v.size();
			i1* rv = (i1*) &(state->v)[0];
			return rv;
		}

		space * is_space(_LuaKeyMap& keys, int at = 1) const {
			if (lua_istable(L, at)) {
				ptrdiff_t pt = reinterpret_cast<ptrdiff_t>(lua_topointer(L, at));
				auto f = keys.find(pt);
				if(f!=keys.end()){
					return f->second;
				}
			}
			return nullptr;
		}
		space * get_space(_LuaKeyMap& keys, int at = 1) {
			space * space = is_space(keys,at);
			if(space==nullptr){
				luaL_error(L, "no key associated with table of type %s", SPACES_LUA_TYPE_NAME);
			}
			return space;
		}
		void to_space
				(	_LuaKeyMap& keys
					,	spaces::key& d
					,   spaces::record& r
					, 	int _at = 2
				) {
			int at = _at;
			if (at < 0)
				at = lua_gettop(L) + (1 + at);//translate to absolute position
			int lt = lua_type(L, at);
			switch (lt) {
				case LUA_TTABLE: {
					auto space = is_space(keys,at);
					if(space!=nullptr){
						if (space->second.get_identity()) {
							r.set_identity(space->second.get_identity()); // a link has no identity of its own ???
						}
						break;
					}
					/// Seems like a compiler bug if this-> isn't used then all sorts of errors
					this->resolve_id(d,r,true); /// assign an identity to p (its leaves will need it)
					lua_pushnil(L);
					/// will push the name and value of the current item (as returned by closure)
					/// on the stack
					while (lua_next(L, at) != 0) {
						typename _Self::space s;
						s.first.set_context(r.get_identity()); //
						to_space_data(s.first.get_name(), -2);	// just set the key part
						to_space(keys,s.first, s.second, -1); // d becomes a parent and will receive an id if -1 is another table
						/// add new key to table
						/// Seems like a compiler bug if this-> isn't used then all sorts of errors
						this->insert_or_replace(s);
						lua_pop(L, 1);
					}

				}break;
				case LUA_TUSERDATA: {
					/// add the link here
					space * l = err_checkudata<space>(L, SPACES_LUA_TYPE_NAME, at);
					if (l == nullptr) break;
					if (l->second.get_identity()) {
						r.set_identity(l->second.get_identity()); // a link has no identity of its own ???
					}

				}break;
				case LUA_TFUNCTION: {

					typename _Self::_DumpVector dv;

					lua_pushvalue(L, at);
					if (0 != lua_dump(L, vector_writer<_Self::_DumpVector>, &dv)) {
						lua_error(L);
					}
					lua_pop(L, 1);
					r.get_value().set_function(dv);

				}break;
				case LUA_TNIL:
					/// delete space under parent using name
				{
					//d.set_context(p.get_identity());
					session_type::session.get_set().erase(d);
					/// TODO: collect garbage anyone, its wednesday
				}break;
				default:
					to_space_data(r.get_value(), at);
					break;
			};
		}

		space* open_space(_LuaKeyMap& keys, ui8 id) {
			i4 t = lua_gettop(L);
			if (!is_space(keys, -1)) {
				lua_newtable(L);
				space* r = new space(); // spaces::create_key_from_nothing(L);
				// set its metatable
				luaL_getmetatable(L, SPACES_LUA_TYPE_NAME);
				if (lua_isnil(L, -1)) {
					luaL_error(L, "no meta table of type %s", SPACES_LUA_TYPE_NAME);
				}
				lua_setmetatable(L, -2);
				ptrdiff_t pt = reinterpret_cast<ptrdiff_t>(lua_topointer(L, -1));
				keys[pt] = r;
				r->session = this;
				return r;
			}
			else {
				space* r = this->get_space(keys,-1);
				r->session = this;
				return r;
			}
		}


		template<typename _VectorType>
		static int vector_writer(lua_State *L, const void* p, size_t sz, void* ud) {
			auto* v = (_DumpVector*)ud;
			v->insert(v->end(), (const _DumpVector::value_type*)p, ((const _DumpVector::value_type*)(p)) + sz);
			return 0;
		}

		void to_space_data
		(	 spaces::data& d
			, int _at = 2
		) {
			int at = _at;
			if (at < 0)
				at = lua_gettop(L) + (1 + at);//translate to absolute position
			int lt = lua_type(L, at);
			switch (lt) {
			case LUA_TNUMBER: {
				double n = ::lua_tonumber(L, at);
				d = n;
			}break;
			case LUA_TSTRING: {
				size_t l = 0;
				const char * v = ::lua_tolstring(L, at, &l);
				d.set_text(v, l);
			}break;
			case LUA_TBOOLEAN: {
				bool b = (lua_toboolean(L, at) != 0);
				d = b;
			}break;
			case LUA_TTABLE: {

				/// TODO: multi data format

			}break;
			case LUA_TUSERDATA: {

				/// TODO: multi data format

			}break;
			case LUA_TFUNCTION: {

				typename _Self::_DumpVector dv;

				lua_pushvalue(L, at);
				if (0 != lua_dump(L, vector_writer<typename _Self::_DumpVector>, &dv)) {
					lua_error(L);
				}
				lua_pop(L, 1);
				d.set_function(dv);

			}break;
			case LUA_TNIL:
			{
				d.clear();
			}break;
			default:
				luaL_error(L, "could not convert lua type to space");
				break;
			};
		}

		int push_data(const spaces::data& d) {
			switch (d.get_type()) {
			case data_type::numeric:
				lua_pushnumber(L, d.to_number());
				break;
			case data_type::boolean:
				lua_pushboolean(L, (int)d.get_integer());
				break;
			case data_type::text:
				lua_pushlstring(L, d.get_sequence().c_str(), d.get_sequence().size());
				break;
			case data_type::function:
				lua_pushnil(L);
				break;
			case data_type::infinity:
				lua_pushnumber(L, std::numeric_limits<lua_Number>::infinity());
				break;
			default:
				lua_pushnil(L);
			}
			return 1;
		}
		int push_data(const spaces::record& d) {
			return push_data(this->map_data(d).get_value());
		}


		void to_space
		(	_LuaKeyMap& keys
		,	space& d
		, 	int _at = 2
		) {
			to_space(keys, d.first,d.second,_at);
		}
		int push_pair(_LuaKeyMap& keys, const spaces::key& k,const spaces::record& v) {
			push_data(k.get_name());
			if (v.get_identity() != 0) {
				space * r = this->open_space(keys,v.get_identity());
				r->first = k;
				r->second = v;
			}
			else {
				push_data(v.get_value());
			}
			return 2;
		}
		int push_space(const space& s) {
			return push_pair(s.first, s.second);
		}
	}; /// lua_session
	typedef lua_session<db_session> session_t;
	typedef session_t::space space;



}


//extern void register_spaces_replication();
#ifdef _MSC_VER
extern "C" int __declspec(dllexport) luaopen_spaces(lua_State * L);
extern "C" int __declspec(dllexport) luaclose_spaces(lua_State * L);
#else
extern "C" int luaopen_spaces(lua_State * L);
extern "C" int luaclose_spaces(lua_State * L);
#endif
//extern int r_initialize_spaceslib(lua_State *L,DSASession* session = NULL);