//
// Created by pirow on 2018/05/19.
//

#ifndef SPACES_SPACES_SESSION_H
#define SPACES_SPACES_SESSION_H
#include <limits>
#include <vector>
#include <string>
#include <set>
#include <storage/spaces/key.h>
#include <storage/spaces/dbms.h>
#include <rabbit/unordered_map>

namespace spaces{
    typedef rabbit::unordered_map<ptrdiff_t, spaces::space*> _LuaKeyMap;
    //typedef spaces::dbms::_Set _SSet;
    //typedef _SSet::iterator iterator;
    typedef std::map<key,record> _MMap;


    struct db_session {
        typedef spaces::dbms::_Set _Set;
        //spaces::_SSet s;
        std::shared_ptr<spaces::dbms> d;
        bool is_reader;
        db_session(bool is_reader = false) : is_reader(is_reader){
            this->create();

        }
        ~db_session(){

        }
        void create(){
            if(is_reader){
                d =spaces::create_reader();
            }else{
                d = spaces::get_writer();
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
        void set_mode(bool reader){
            if(is_reader != reader){
                if(d!= nullptr) d->rollback();
                d = nullptr;
                is_reader = reader;
                create();
            }
        }
        void begin() {
            d->begin();
        }
        void commit() {
            d->commit();
        }

    };
    struct mem_session {
        spaces::_MMap s;
        typedef spaces::_MMap _Set;
        nst::u64 id;
        mem_session(bool) : id(1) {
        }

        _Set &get_set() {
            return s;
        }
        nst::u64 gen_id(){
            return id++;
        }
        void set_mode(bool) {

        }
        void check(){

        }
        void begin() {

        }
        void commit() {
        }

    };
    template<typename _Set>
    struct spaces_iterator {
        typename _Set::iterator i;
        typename _Set::iterator e;
        bool end() const {
            return i == e;
        }
        void next() {
            ++i;
        }
    };

    static const spaces::record& get_data(const spaces::mem_session::_Set::iterator& i){

        return (*i).second;
    }
    static spaces::record& get_data(spaces::mem_session::_Set::iterator& i){
        return (*i).second;
    }
//static const spaces::record& get_data(const spaces::db_session::_Set::iterator& i){
//	return i.data();
//}
    static spaces::record& get_data(spaces::db_session::_Set::iterator& i){
        return i.data();
    }


    static const spaces::key& get_key(const spaces::mem_session::_Set::iterator& i){
        return (*i).first;
    }
//static spaces::key& get_key(spaces::mem_session::_Set::iterator& i){
//	return (*i).first;
//}
    static const spaces::key& get_key(const spaces::db_session::_Set::iterator& i){
        return i.key();
    }
    static spaces::key& get_key(spaces::db_session::_Set::iterator& i) {
        return i.key();
    }
    static ptrdiff_t get_count(const spaces::db_session::_Set::iterator& i, const spaces::db_session::_Set::iterator& j) {
        return i.count(j);
    }
    static ptrdiff_t get_count(const spaces::mem_session::_Set::iterator& i, const spaces::mem_session::_Set::iterator& j) {
        ptrdiff_t cnt = 0;
        spaces::mem_session::_Set::iterator ii = i;

        while(ii != j){
            ++cnt;
            ++ii;
        }
        return cnt;

    }

    template<typename _SessionType>
    class spaces_session {
    protected:

        _SessionType session;
        spaces::key data_key;
        spaces::record large;

    public:

        typedef typename _SessionType::_Set _Set;
        spaces_session(bool reader) : session(reader) {

        }
        ~spaces_session() {

        }
        void set_max_memory_mb(const ui8& mm){
            session.set_max_memory_mb(mm);
        }

        void set_mode(bool readr){
            session.set_mode(readr);
        }
        void check(){
            session.check();
        }
        void begin() {
            session.begin();
        }
        void commit() {
            session.commit();
        }
        void rollback() {

        }

        typename _SessionType::_Set& get_set() {
            return session.get_set();
        }

        nst::u64 len(const space* p) {
            if (p->second.get_identity() > 0) {
                spaces::key f, e;
                f.set_context(p->second.get_identity());
                e.set_context(p->second.get_identity());
                e.get_name().make_infinity();
                ///.set_identity(std::numeric_limits<ui8>::max());
                auto& s = get_set();
                // create the iterator
                typename _Set::iterator fi = s.lower_bound(f);
                if (fi != s.end()) {
                    return get_count(fi,s.upper_bound(e));

                }

            }
            return 0;
        }

        void insert_or_replace(spaces::key& k, spaces::record& v) {
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
                    session.get_set()[data_key] = data;
                }

                value.clear();

            }
            session.get_set()[k] = v;
        }
        void insert_or_replace(spaces::space& p) {
            insert_or_replace(p.first,p.second);
        }
        void resolve_id(spaces::space* p, bool override = false) {
            resolve_id(p->first, p->second,override);
        }

        template<typename _DataType>
        spaces::record& map_data(_DataType& original){

            if(original.is_flag(spaces::record::FLAG_LARGE)){
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

        void resolve_id(spaces::key& first, spaces::record& second, bool override = false) {
            if (override || second.get_identity() == 0) {
                auto& s = session.get_set();
                auto i = s.find(first);
                if (i != s.end()) {
                    second = this->map_data(get_data(i));
                }
                if (override || second.get_identity() == 0) {
                    second.set_identity(session.gen_id()); /// were gonna be a parent now so we will need an actual identity
                    insert_or_replace(first,second);
                }
            }
        }

        void resolve_id(spaces::space& p, bool override = false) {
            resolve_id(&p);
        }

    };
}


#endif //SPACES_SPACES_SESSION_H
