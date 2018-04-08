#pragma once
#include <string>
#include <algorithm>
#include <iterator>
#include <limits>
#include <storage/spaces/data_type.h>
#include <stx/storage/basic_storage.h>
#ifdef _MSC_VER
#define SPACES_NOINLINE_PRE _declspec(noinline)
#define SPACES_NOINLINE_
#else
#define SPACES_NOINLINE_PRE
#define SPACES_NOINLINE_ __attribute__((noinline))
#endif
namespace spaces {
	namespace nst = stx::storage;
	struct data_type {
		
		enum {
			numeric,
			boolean,
			text, 
			function,
			multi,
			infinity
		};
	};
	class astring {
	private:
		static const i4 SS = 256;
		i4 l;
		i1 sequence[sizeof(std::vector<char>)];
		std::vector<char> & make_long() {
			new (sequence) std::vector<char>();
			l = SS;
			return (std::vector<char>&)sequence;
		}

		std::vector<char>&str() {
			return (std::vector<char>&)sequence;
		}
		const std::vector<char> &str() const {
			return (std::vector<char>&)sequence;
		}
		void resize(i4 l, const char * data) {
			if (l >= sizeof(sequence)) {
				std::vector<char> &s = make_long();
				s.resize(l);
				memcpy(&s[0], data, l);
				return;
			}
			this->l = l;
			memcpy(sequence, data, l);

		}
		SPACES_NOINLINE_PRE /// assume the comparison of long strings will happen less often 
		i4 compare_data_long(const astring& right) const
		SPACES_NOINLINE_ /// other compilers
		{
			return memcmp(data(), right.data(), std::min<i4>(size(), right.size()));
		}
	public:
		void clear() {
			if (is_long()) {
				str().clear();
			}
			else {
				l = 0;
			}
		}
		bool is_long() const {
			return l == SS;
		}
		void resize(i4 l) {
			if (l >= sizeof(sequence)) {
				make_long().resize(l);					
				return;
			}			
			this->l = l;
		}		
		void set_data(const char * data,i4 l) {
			resize(l,data);			
		}
		
		i4 size() const {
			return l == SS ? (i4)(str().size()) : 
				l;
		}
		const char *data() const {
			return l == SS ? str().data() : 
				sequence;
		}
		const char *c_str() const {
			return l == SS ? str().data() :
				sequence;
		}
		char *writable(){
			return l == SS ? &str()[0] : 
				sequence;
		}
		const char *readable() const {
			return l == SS ? &str()[0] :
				   sequence;
		}
		
		i4 compare(const astring& right) const {
			i4 r = 0;
			if (right.l != SS && l != SS) {
				r = memcmp(sequence, right.sequence, std::min<i4>(l, right.l));				
			}
			else {
				r = compare_data_long(right);
			}
			if (r == 0) {
				return l - right.l;
			}
			return r;		
		}
		astring() : l(0){

		}
		astring(const astring& right) : l(0) {
			*this = right;
		}
		~astring() {
			if (is_long()) {
				//str().~std::vector<char>();
			}
		}
		astring& operator=(const astring& right) {
			if (!right.is_long()) {
				set_data(right.sequence, right.l);
			}
			else {
				set_data(right.data(), right.size());
			}
			return *this;
		}
		astring& operator=(const std::string& right) {
			
			set_data(right.data(), (i4)right.size());
			return *this;
		}

	};
	class data {
	private:
		i4 type;
		astring sequence;
	public:
		data(const data&r)
		: 	sequence(r.sequence)
		, 	type(r.type){

		}
		void clear() {
			type = data_type::numeric;
			sequence.clear();
		}
		data() {
			type = data_type::numeric;

			
		}
		template<typename T>
		T& cast_sequence(){
            sequence.resize(sizeof(T));

            return *(T*)(sequence.writable());
		}
		template<typename T>
		const T& cast_sequence() const {
			return *(T*)(sequence.readable());
		}

		double& get_double(){
			return cast_sequence<double>();
		}
		double& get_number(){
			return cast_sequence<double>();
		}
		i8& get_integer(){
			return cast_sequence<i8>();
		}
		const double& get_double() const {
			return cast_sequence<double>();
		}
		const double& get_number() const{
			return cast_sequence<double>();;
		}
		const i8& get_integer() const{
			return cast_sequence<i8>();
		}
		data& operator=(const data& r) {
			type = r.type;
			sequence = r.sequence;
			return *this;
		}
		data& operator=(const ui8& r) {
			clear();
			type = data_type::numeric;
			return *this;
		}
		data& operator=(const bool& r) {
			clear();
			type = data_type::boolean;
			get_integer() = r;
			return *this;
		}
		data& operator=(const double& r) {
			clear();
			type = data_type::numeric;
			get_number() = r;
			return *this;
		}
		data& operator=(const std::string& r) {
			clear();
			type = data_type::text;			
			sequence = r;
			return *this;
		}
		void make_infinity() {
			clear();
			type = data_type::infinity;
			get_number() = std::numeric_limits<f8>::infinity();
		}
		void set_text(const i1* s, const size_t l) {
			clear();
			type = data_type::text;
			sequence.set_data(s, (i4)l);
			
		}
		template<typename _VectorType>
		void set_function(const _VectorType& vt) {
			clear();
			type = data_type::function;
			//std::copy(vt.begin(), vt.end(), std::back_inserter(sequence));
			//sequence.append(s, l);

		}
		i4 compare(const data&right) const {
			if (type != right.type) return type - right.type;
			if (is_text()) {				
				return sequence.compare(right.sequence);
			}else {			
				return (int)(get_number() - right.get_number());
			}
			return 0;
		}
		bool operator != (const data&r) const {
			return compare(r) != 0;
		}
		bool operator < (const data& r) const {
			return compare(r) < 0;
			
		}
		char* get_sequence() {
			return sequence.writable();
		}

		const astring& get_sequence() const {
			return sequence;
		}
		bool is_text() const {
			return type == data_type::text;
		}
		f8 to_number() const {
			char* end;
			switch (type) {
			case data_type::numeric:
				return get_number();
			case data_type::boolean:
				return get_number();
			case data_type::text:
				return std::strtod(this->sequence.c_str(), &end);
			case data_type::function:				
			case data_type::multi:
			case data_type::infinity:
			default:
				break;
			}
			return std::numeric_limits<f8>::quiet_NaN();
		}
		i4 get_type() const {
			return this->type;
		}
		nst::u32 stored() const {
			nst::i32 ts = sizeof(ui8);
			nst::i32 ss = sequence.size() + sizeof(sequence.size());
			nst::i32 ns = sizeof(ui8);
			return ts + (this->is_text() ? ss : ns);
		};
		
		nst::buffer_type::iterator store(nst::buffer_type::iterator w) const {
			nst::buffer_type::iterator writer = nst::primitive::store(w, (ui8)this->type);
			if (this->is_text()) {
				writer = nst::primitive::store(writer, sequence.size());
				memcpy((nst::u8*)&(*writer), sequence.data(), sequence.size());
				writer += sequence.size();
			}
			else {
				writer = nst::primitive::store(writer, get_integer());
			}
			if (writer - w != stored()) {
				ptrdiff_t diff = writer - w;
				ptrdiff_t stred = stored();
				err_print("wrote wrong byte count");
			}
			return writer;
		};

		nst::buffer_type::const_iterator read(const nst::buffer_type& buffer, nst::buffer_type::const_iterator r) {
			
			nst::buffer_type::const_iterator reader = r;
			ptrdiff_t diff = reader - r;
			if (reader != buffer.end()) {
				ui8 t = 0;
				reader = nst::primitive::read(t, reader);
				this->type = t;
				if (is_text()) {

					auto s = sequence.size();
					reader = nst::primitive::read(s, reader);
					sequence.resize(s);
					memcpy(sequence.writable(), (nst::u8*)&(*reader), sequence.size());
					reader += sequence.size();
				}
				else {
					diff = reader - r;
					reader = nst::primitive::read(this->get_integer(), reader);
					diff = reader - r;
				}
			}
			if (reader - r != stored()) {
				diff = reader - r;
				ptrdiff_t stred = stored();
				err_print("wrote wrong byte count");
			}
			return reader;
		};
	};
	class key {
	private:
		ui8 context;
		data name;
	
	public:
		static const bool use_encoding = true;
		key(const key&r) : context(r.context), name(r.name){

		}
		key() {
			context = 0;
		}
		key& operator=(const key& r) {
			name = r.name;
			context = r.context;			
			return *this;
		}
		~key() {

		}
		bool operator != (const key&r) const {
			if (context != r.context) return true;
			if (name != r.name) return true;
			return false;
		}
		bool operator < (const key&r) const {
			if (context != r.context) return context < r.context;			
			int l = name.compare(r.name);
			if (l < 0) return true;
			return false;
		}
		bool found (const key&r) const {
			if (context != r.context) return false;
			if (name != r.name) return false;
			return true;
		}
		ui8 get_context() const {
			return this->context;
		}
		void set_context(ui8 context) {
			this->context = context;
		}
		data& get_name() {
			return name;
		}
		const data& get_name() const{
			return name;
		}

	public:
		/// persistence functions		
		nst::buffer_type::const_iterator read(const nst::buffer_type& buffer, typename nst::buffer_type::const_iterator& r) {
			nst::buffer_type::const_iterator reader = r;

			if (reader != buffer.end()) {
				if (use_encoding) {
					this->context = nst::leb128::read_unsigned64(reader, buffer.end());

				}
				else {
					reader = nst::primitive::read(this->context, reader);
				}
				reader = name.read(buffer, reader);
			}
			return reader;
		}
		nst::u32 stored() const {
			if (use_encoding) {
				return nst::leb128::unsigned_size(this->context) + name.stored() ;
			}
			else {
				return sizeof(this->context) + name.stored();
			}
			
		}
		nst::buffer_type::iterator store(nst::buffer_type::iterator& w) const {
			nst::buffer_type::iterator writer = w;
			if (use_encoding) {
				writer = nst::leb128::write_unsigned(writer, this->context);
			}
			else {
				writer = nst::primitive::store(writer, this->context);
			}
			
			
			writer = name.store(writer);
			if (writer - w != stored()) {
				ptrdiff_t diff = writer - w;
				ptrdiff_t stred = stored();
				err_print("wrote wrong byte count");
			}
			return writer;
		}
		
	};
	class record {
	private:

		ui8 identity;
		// value section
		data value;

	public:
		static const bool use_encoding = true;
		record(const record&r) {
			*this = r;
		}
		record() {
			identity = 0;
		}
		record& operator=(const record& r) {

			value = r.value;
			identity = r.identity;

			return *this;
		}
		~record() {

		}
		ui8 get_identity() const {
			return this->identity;
		}
		void set_identity(ui8 identity) {
			this->identity = identity;
		}

		data& get_value() {
			return value;
		}
		const data& get_value() const {
			return value;
		}

	public:
		/// persistence functions
		nst::buffer_type::const_iterator read(const nst::buffer_type& buffer, typename nst::buffer_type::const_iterator& r) {
			nst::buffer_type::const_iterator reader = r;

			if (reader != buffer.end()) {
				if (use_encoding) {
					this->identity = nst::leb128::read_unsigned64(reader, buffer.end());
				}
				else {
					reader = nst::primitive::read(this->identity, reader);
				}

				reader = value.read(buffer, reader);
			}
			return reader;
		}
		nst::u32 stored() const {
			if (use_encoding) {
				return nst::leb128::unsigned_size(this->identity) + value.stored() ;
			}
			else {
				return sizeof(this->identity) + value.stored();
			}

		}
		nst::buffer_type::iterator store(nst::buffer_type::iterator& w) const {

			nst::buffer_type::iterator writer = w;
			if (use_encoding) {
				writer = nst::leb128::write_unsigned(writer, this->identity);
			}
			else {
				writer = nst::primitive::store(writer, this->identity);
			}
			writer = value.store(writer);
			if (writer - w != stored()) {
				ptrdiff_t diff = writer - w;
				ptrdiff_t stred = stored();
				err_print("wrote wrong byte count");
			}
			return writer;
		}

	};
	typedef std::pair<key, record> space;
}