//
// Created by pirow on 2018/05/19.
//

#ifndef SPACES_SPACES_SESSION_H
#define SPACES_SPACES_SESSION_H
#include <limits>
#include <vector>
#include <string>
#include <set>
#include <stx/storage/types.h>
#include <storage/spaces/key.h>
#include <storage/spaces/dbms.h>
#include <rabbit/unordered_map>

namespace spaces{
    static void dbg_space(const char * prefix,const spaces::key& k, const spaces::record& v){
        dbg_print("%s [ctx:%lld, name:'%s', id:%lld, val:'%s']",prefix,(nst::lld)k.get_context(),k.get_name().to_string().c_str(),(nst::lld)v.get_identity(),v.get_value().to_string().c_str());
    }

	extern std::atomic<long> db_session_count;
    typedef std::map<key,record> _MMap;

    /**
     * the database session
     */
    struct db_session {
        typedef spaces::dbms::_Set _Set;
        spaces::dbms::ptr d;
        std::string name;
        //bool is_reader;
        db_session(const std::string& name, bool is_reader = false) {
            this->name = name;
			++db_session_count;
            this->create(name);

        }
        ~db_session(){
            dbg_print("destroy db session %s",this->name.c_str());
			--db_session_count;
        }
        const std::string& get_name() const {return this->name;}
        spaces::dbms::ptr get_dbms(){
            return d;
        }
        bool is_transacted() const{
            return d->is_transacted();
        }
        void create(const std::string& name, bool reader = true){
            if(reader){
                d = spaces::create_reader(name);
                if(!d->reader()){
                    err_print("retrieved dbms in invalid mode");
                }
            }else{
                d = spaces::get_writer(name);
                if(d->reader()){
                    err_print("retrieved dbms in invalid mode");
                }
            }
        }

        _Set &get_set() {
            return d->get_set();
        }
        nst::u64 gen_id(){
            return d->gen_id();
        }
        void check(){
            d->check_resources();
        }

        void begin(bool reader) {
            if(d!=nullptr){
                if(!reader && d->reader()){
                    d->rollback();
                    create(this->name,reader);
                }
            }else{
                create(this->name,reader);
            }

            d->begin();
        }
        void commit() {
            d->commit();
        }
        void rollback() {
            d->rollback();
        }
        const bool reader() const {
            return d->reader();
        }


    };
    extern nst::lld iterator_count;
    /**
     * iterator for space container
     * @tparam _Set the set type which will supply elements to iterate
     */
    template<typename _Set>
    struct spaces_iterator {
        key upper;
        key lower;

        nst::i64 position;
        spaces_iterator(): position(0){++iterator_count;}
        ~spaces_iterator(){
            --iterator_count;
        }
        void set_upper(_Set& s,const  key& upper){
            this->e = s.lower_bound(upper);
            this->upper = upper;
        }
        void set_lower(_Set& s,const  key& lower){
            this->s = s.lower_bound(lower);
            this->i = this->s;
            this->lower = lower;
        }
        void recover(_Set& s){
            set_upper(s,this->upper);
            set_lower(s,this->lower);

            this->i += position;
            this->i.validate();
            this->s.validate();

        }
        void last(){
            position = i.count(e);
            i = e;
            --i;

        }
        bool is_valid(){

            return !i.has_changed() && !s.has_changed();
        }
        const typename _Set::iterator& get_i() const {
            return i;
        }
        long long count() const {
            return s.count(e);
        }
        const typename _Set::iterator& get_at(long long index) {
            t = i;

            t.advance(index,this->e);
            return t;
        }
        const typename _Set::iterator& get_last() {
            t = e;
            --t;
            return t;
        }
        const typename _Set::iterator& get_first() {
            return s;
        }
        void move(long long index){
            i.advance(index,this->e);
            position += index;
        }
        bool end() const {
            return i == e;
        }
        bool empty() const {
            return s == e;
        }
        bool not_end() const {
            return i != e;
        }
        bool not_start() const {
            return i != s;
        }
        bool first() const{
            return is_start();
        }
        bool is_start() const {
            return i == s;
        }
        void next() {
            ++i;
            ++position;
        }
        void previous() {
            --i;
            --position;
        }
        void start(){
            this->i = s;
            position = 0;
        }

    private:
        typename _Set::iterator i;

        typename _Set::iterator s;

        typename _Set::iterator e;

        typename _Set::iterator t;

    };

    static spaces::record& get_data(spaces::db_session::_Set::iterator& i){
        return i.data();
    }
    static const spaces::record& get_data(const spaces::db_session::_Set::iterator& i){
        return i.data();
    }
    static const spaces::key& get_key(const spaces::db_session::_Set::iterator& i){
        return i.key();
    }
    static spaces::key& get_key(spaces::db_session::_Set::iterator& i) {
        return i.key();
    }
    static ptrdiff_t get_count(const spaces::db_session::_Set::iterator& i, const spaces::db_session::_Set::iterator& j) {
        return i.count(j);
    }
    /**
     * the session keeps storage and transactional state as well
     * as enforcing some relational integrity
     * @tparam _SessionType the base session type - can be associated
     * with a specific language implementation
     */
    template<typename _SessionType>
    class spaces_session {
    public:
        typedef std::shared_ptr<spaces_session> ptr;


        _SessionType session;
        spaces::key data_key;
        spaces::record large;


    protected:
    public:
        typedef session_space<spaces_session> space;
        typedef typename _SessionType::_Set _Set;
        /**
         * construct session specifying initial transaction mode
         * @param name of session storage
         * @param reader transaction mode - true for reader
         */
        spaces_session(const std::string& name, bool reader) : session(name,reader) {

        }
        ~spaces_session() {
            dbg_print("destroying spaces session");
        }
        /**
         *
         * @return the session internal type
         */
        _SessionType& get_session_type(){
            return session;
        }
        /**
         * return the storage name associated with session dbms
         * @return storage name for session
         */
        const std::string& get_name() const {
            return session.get_name();
        }
        /**
         *
         * @return the database management system for this session
         */
        spaces::dbms::ptr get_dbms(){
            return session.get_dbms();
        }
        /**
         * set the global memory limit in megabytes
         * @param mm limit megabytes
         */
        void set_max_memory_mb(const ui8& mm){
            session.set_max_memory_mb(mm);
        }

        /**
         * start a transaction in writing mode
         */
        void begin_writer(){

            this->begin(false);
        }
        void begin_reader(){

            this->begin(true);
        }
        void check(){
            session.check();
        }
        void begin(bool reader) {
            session.begin(reader);
        }
        void commit() {
            session.commit();
        }
        void rollback() {
            session.rollback();
        }
        const bool reader() const {
            return session.reader();
        }
        typename _SessionType::_Set& get_set() {
            if(!session.is_transacted()){
                err_print("transaction not started");
                throw std::exception();
            }
            return session.get_set();
        }

        std::pair<bool,typename _Set::iterator> last(const space* p) {
            auto& s = get_set();

            auto identity = p->second.get_identity();
            if (s.size() > 0 && identity > 0) {
                spaces::key e;
                e.set_context(identity);
                e.get_name().make_infinity();
                typename _Set::iterator li = s.upper_bound(e);
                --li;
                if(li.key().get_context() == identity)
                    return std::make_pair(true,li);



            }
            return std::make_pair(false,typename _Set::iterator());
        }

        std::pair<bool,typename _Set::iterator> first(const space* p) {
            auto& s = get_set();
            auto identity = p->second.get_identity();
            if (s.size() > 0 && identity > 0) {
                spaces::key f;
                f.set_context(identity);
                typename _Set::iterator fi = s.lower_bound(f);
                if(fi != s.end() && fi.key().get_context() == identity)
                    return std::make_pair(true,fi);
            }
            return std::make_pair(false,typename _Set::iterator());
        }
        nst::u64 len(const space* p) {
            if (p->second.get_identity() > 0) {
                spaces::key f, e;
                f.set_context(p->second.get_identity());
                e.set_context(p->second.get_identity());
                e.get_name().make_infinity();
                auto& s = get_set();
                // create the iterator
                typename _Set::iterator fi = s.lower_bound(f);
                if (fi != s.end()) {
                    return get_count(fi,s.upper_bound(e));

                }

            }
            return 0;
        }
        void erase(spaces::key& k){
            session.get_set().erase(k);
        }
        /**
         * insert a key value pair applying fragmentation for large buckets
         * @param k the key
         * @param v value with data
         */
        void insert_or_replace(const spaces::key& k, spaces::record& v) {
            const ui4 MAX_BUCKET = 100;
            if(v.size() >  MAX_BUCKET){
                v.set_flag(spaces::record::FLAG_LARGE);
                spaces::key data_key;
                spaces::record data;
                v.set_identity(session.gen_id());

                auto& value = v.get_value();
                ui4 written = 0;
                ui4 left = value.size();
                ui4 index = 0;
                const char * val = value.get_sequence().readable();
                while(left > 0){
                    ui4 todo = std::min<ui4>(left,value.size());
                    data.get_value().set_text(val,todo);
                    data_key.get_name() = (ui8)index;
                    data_key.set_context(v.get_identity());
                    left -= todo;
                    ++index;
                    dbg_space("inserting data",data_key,data);
                    session.get_set()[data_key] = data;
                }

                value.clear();

            }
            dbg_space("inserting",k,v);
            session.get_set()[k] = v;

        }
        void insert_or_replace(space& p) {
            insert_or_replace(p.first,p.second);
        }
        /**
         *
         * @param p
         * @param override
         */
        void resolve_id(space* p, bool override = false) {
            resolve_id(p->first, p->second,override);
        }
        /**
         * links another space (other) with a value originating in this session or storage
         * assigns a external name to the value of local and sets the ROUTE flag
         * if other originates externally
         * @tparam _OtherSessionType
         * @param local the value object belonging to this session and storage
         * @param other the session space that belongs to potentialy another session
         * @return true if something has been linked
         */
        template<class _OtherSessionType>
        bool link(spaces::record& local, const _OtherSessionType* other){
            if(other->second.get_identity()) {
                local.set_identity(other->second.get_identity()); // a link has no identity of its own ???

                // here we will differentiate between native and external spaces
                // external spaces are not always remote
                auto os = other->get_session();
                if(this->get_name()!= os->get_name()){

                    local.set_value(os->get_name());
                    local.set_flag(spaces::record::FLAG_ROUTE);
                }
                return true;
            }
            return false;
        }
        /**
         * maps large data into memory by check FLAG_LARGE and then collecting
         * fragments
         * @tparam _DataType
         * @param original
         * @return
         */
        template<typename _DataType>
        spaces::record& map_data(_DataType& original){

            if(original.is_flag(spaces::record::FLAG_LARGE)){
                dbg_print("mapping large data");
                spaces::key data_key;
                ui4 index = 0;
                std::string buffer;
                data_key.get_name() = (ui8) index;
                data_key.set_context(original.get_identity());
                auto datas = get_set().find(data_key);
                while (datas != get_set().end() && get_key(datas).get_context() == original.get_identity()) {
                    const spaces::record& d = get_data(datas);
                    auto& val = d.get_value();
                    auto& seq = val.get_sequence();
                    buffer.append(seq.readable(),seq.size());
                    ++datas;
                }
                large.set_identity(0);
                large.get_value().set_string(buffer);
                return large;

            }
            return const_cast<spaces::record&>(original);
        }
        /**
         * sets an id in second if its 0
         * starts by first finding the value in the local storage to check that the
         * id has not been set before since where dealing with a representation
         * if the identity has not been set then assign a new identity and store it.
         * other transactions will only see this change if they started
         * after this transaction committed
         * read only transactions will fail -
         * it is contract that this operation will be avoided in read only transactions
         * and thus cause an unrecoverable error.
         * This operation is fairly expensive due to all the due diligence and complexity of
         * the linking contract.
         * @param first the key to order within storage
         * @param second the value which will receive an id
         * @param override force an id into second even if one is found this will orphan the children
         *
         */
        void resolve_id(const spaces::key& first, spaces::record& second, bool override = false) {


            if (override || second.get_identity() == 0) {
                auto& s = session.get_set();
                auto i = s.find(first);
                if (i != s.end()) {
                    second = this->map_data(get_data(i)); /// update to the latest version
                }
                if (override || second.get_identity() == 0) {

                    auto nid = session.gen_id();
                    if(override && second.get_identity() != 0){
                        dbg_print("storage [%s] space [%s] overriding id: %lld -> %lld",this->get_name().c_str(),first.get_name().to_string().c_str(),(nst::lld)second.get_identity(),(nst::lld)nid);
                    }
                    second.set_identity(nid); /// were gonna be a parent now so we will need an actual identity
                    insert_or_replace(first,second);

                }
            }

            dbg_print("storage [%s] space [%s] resolved id: %lld",this->get_name().c_str(),first.get_name().to_string().c_str(),(nst::lld)second.get_identity());
        }

        /**
         *
         * @param p the space that will receive an id
         * @param override force a new id even if one is found
         */
        void resolve_id(space& p, bool override = false) {
            resolve_id(&p);
        }

    };
    template<class _SessionResultType>
    class sessions_map {
    public:
        typedef typename _SessionResultType::ptr ptr;
        /**
        * sessions map to
        */
        typedef rabbit::unordered_map<std::string, ptr> sessions_type;
        typedef std::shared_ptr<sessions_type> sessions_ptr;
    private:
        sessions_type sessions;
        sessions_type& get_sessions(){
            return sessions;
        }
        const sessions_type& get_sessions() const {
            return sessions;
        }
    public:
        /**
         * this object is not copyable
         * @return
         */
        sessions_map & operator=(const sessions_map&) = delete;
        sessions_map(const sessions_map&) = delete;
        sessions_map(){

        }

        /**this->get_name()
         * create a new or return an existing session with the named storage
         * 'storage' and register it in internal sessions map
         * @tparam _SessionResultType
         * @param self the shared pointer to this
         * @param storage name of storage to use
         * @return pointer of new session with same type as self
         */

        typename _SessionResultType::ptr create_session(ptr caller,const std::string& storage){
            auto s = this->get_sessions().find(storage);
            if(s != this->get_sessions().end()){
                return std::static_pointer_cast<_SessionResultType>(s->second);
            }
            bool reader = caller==nullptr ? true : caller->reader();
            auto result =  std::make_shared<_SessionResultType>(storage,reader);
            this->get_sessions()[storage] = result;
            return result;
        }
        bool remove(const std::string& storage){
            auto s = this->get_sessions().find(storage);
            size_t before = this->size();
            if(s != this->get_sessions().end()){
                this->get_sessions().erase(s);
                size_t after = this->size();
                return true;
            }
            return false;
        }
        size_t size() const {
            return get_sessions().size();
        }
        bool empty() const {
            return get_sessions().empty();
        }

    };
    template<typename _SessionT>
    class state_variables{
    public:
        typedef std::shared_ptr<state_variables> ptr;
        typedef spaces::sessions_map< _SessionT > sessions_type;
    public:
        // disable any copying
        state_variables & operator=(const state_variables&) = delete;
        state_variables(const state_variables&) = delete;

        state_variables(){};
        sessions_type sessions;
        bool empty(){
            return sessions.empty();
        }
    };

#include <storage/transactions/abstracted_storage.h>
    template<typename _TState,typename _SessionT,typename _TSpace>
    class state_cache{
    public:
        typedef state_variables<_SessionT> variables_type;
        typedef rabbit::unordered_map<ptrdiff_t, typename variables_type::ptr> _StateCache;
    private:
        _StateCache states;

    public:
        // disable any copying
        state_cache & operator=(const state_cache&) = delete;
        state_cache(const state_cache& other) = delete;
        state_cache(){}
        void remove_state(_TState *L){
            auto state = get_state(L);
            auto& s = state->sessions;
            if(state->empty()){
                states.erase((ptrdiff_t)L);

            }
            if(states.empty() ){
                stored::stop();
            }
        }
        typename variables_type::ptr get_state(_TState *L){
            typename variables_type::ptr result = nullptr;
            auto s = states.find((ptrdiff_t)L);
            if(s!=states.end()){
                result = s->second;
            }else{
                result = std::make_shared<variables_type>();
                states[(ptrdiff_t)L] = result;
            }
            return result;

        }
        void remove_session(_TState *L,const std::string& storage){
            auto state = get_state(L);
            auto& s = state->sessions;
            s.remove(storage);
            //remove_state(L);

        }
        typename variables_type::sessions_type& get_sessions(_TState *L){
            return get_state(L)->sessions;
        }


        void close_space(_TState *L) {
            dbg_print("Closing spaces key... ");
            //auto& keys = state->keys;

            remove_state(L);
        }

    };
};


#endif //SPACES_SPACES_SESSION_H
