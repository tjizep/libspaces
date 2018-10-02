// $Id: btree_map.h 130 2011-05-18 08:24:25Z tb $
/** \file btree_map.h
 * Contains the specialized B+ tree template class btree_map
 */

/*
 * STX B+ Tree Template Classes s.9.0
 * Copyright (C) 2008-2011 Timo Bingmann
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef _STX_BTREE_MAP_H_
#define _STX_BTREE_MAP_H_
#ifdef _MSC_VER
#pragma warning (disable : 4503)
#endif
#include <stx/btree.h>

namespace stx
{

/** @brief Specialized B+ tree template class implementing STL's map container.
 *
 * Implements the STL map using a B+ tree. It can be used as a drop-in
 * replacement for std::map. Not all asymptotic time requirements are met in
 * theory. The class has a traits class defining B+ tree properties like slots
 * and self-verification. Furthermore an allocator can be specified for tree
 * nodes.
 *
 * Most noteworthy difference to the default red-black implementation of
 * std::map is that the B+ tree does not hold key and data pair together in
 * memory. Instead each B+ tree node has two arrays of keys and data
 * values. This design directly generates many problems in implementing the
 * iterator's operator's which return value_type composition pairs.
 */
template <	typename _Key, typename _Data,
			typename _Storage ,
			typename _Compare = std::less<_Key>,
			typename _Alloc = std::allocator<std::pair<_Key, _Data> > ,
            typename _Traits = bt_def_map_t<_Key, _Data, def_p_traits>,
            typename _Interpolator = stx::interpolator<_Key>
>
class btree_map
{
public:
    // *** Template Parameter Types

    /// First template parameter: The key type of the btree. This is stored in
    /// interior nodes and leaves
    typedef _Key                        key_type;

    /// Second template parameter: The data type associated with each
    /// key. Stored in the B+ tree's leaves
    typedef _Data                       data_type;

	typedef _Data                       mapped_type;

    /// Third template parameter: Key comparison function object
    typedef _Compare                    key_compare;

	/// Fourth template parameter: interpolator if applicable
	typedef _Interpolator				key_interpolator;

    /// Fifth template parameter: Traits object used to define more parameters
    /// of the B+ tree
    typedef _Traits                     traits;

    /// Sixth template parameter: STL allocator

    typedef _Alloc                      allocator_type;

    /// Seventh template parameter: persistence context

    typedef _Storage					storage_type;

    ///typedef _Context					persist_context;



public:
    // *** Constructed Types

    /// Typedef of our own type
    typedef btree_map<key_type, data_type, storage_type, key_compare, allocator_type, traits, key_interpolator> self;

    /// Construct the STL-required value_type as a composition pair of key and
    /// data types
    typedef std::pair<key_type, data_type>      value_type;

    /// Implementation type of the btree_base
    typedef stx::btree<key_type, data_type, storage_type, value_type, key_compare, key_interpolator, traits, false, allocator_type> btree_impl;

    /// Function class comparing two value_type pairs.
    typedef typename btree_impl::value_compare  value_compare;

    /// Size type used to count keys
    typedef typename btree_impl::size_type      size_type;

    /// Small structure containing statistics about the tree
    typedef typename btree_impl::tree_stats     tree_stats;

public:
    // *** Static Constant Options and Values of the B+ Tree

    /// Base B+ tree parameter: The number of key/data slots in each surface
    static const unsigned short         surfaceslotmax =  btree_impl::surfaceslotmax;

    /// Base B+ tree parameter: The number of key slots in each interior node,
    /// this can differ from slots in each surface.
    static const unsigned short         interiorslotmax =  btree_impl::interiorslotmax;

    /// Computed B+ tree parameter: The minimum number of key/data slots used
    /// in a surface. If fewer slots are used, the surface will be merged or slots
    /// shifted from it's siblings.
    static const unsigned short         minsurfaces = btree_impl::minsurfaces;

    /// Computed B+ tree parameter: The minimum number of key slots used
    /// in an interior node. If fewer slots are used, the interior node will be
    /// merged or slots shifted from it's siblings.
    static const unsigned short         mininteriorslots = btree_impl::mininteriorslots;

    /// Debug parameter: Enables expensive and thorough checking of the B+ tree
    /// invariants after each insert/erase operation.
    static const bool                   selfverify = btree_impl::selfverify;

    /// Debug parameter: Prints out lots of debug information about how the
    /// algorithms change the tree. Requires the header file to be compiled
    /// with BTREE_DEBUG and the key type must be std::ostream printable.
    static const bool                   debug = btree_impl::debug;

    /// Operational parameter: Allow duplicate keys in the btree.
    static const bool                   allow_duplicates = btree_impl::allow_duplicates;

public:
    // *** Iterators and Reverse Iterators

    /// STL-like iterator object for B+ tree items. The iterator points to a
    /// specific slot number in a surface.
    typedef typename btree_impl::iterator               iterator;

    /// STL-like iterator object for B+ tree items. The iterator points to a
    /// specific slot number in a surface.
    typedef typename btree_impl::const_iterator         const_iterator;

    /// create mutable reverse iterator by using STL magic
    typedef typename btree_impl::reverse_iterator       reverse_iterator;

    /// create constant reverse iterator by using STL magic
    typedef typename btree_impl::const_reverse_iterator const_reverse_iterator;

private:
    // *** Tree Implementation Object

    /// The contained implementation object
    btree_impl  tree;

public:
    // *** Constructors and Destructor

    /// Default constructor initializing an empty B+ tree with the standard key
    /// comparison function
    explicit inline btree_map
	(	storage_type& storage
	,	const allocator_type &alloc = allocator_type()
	)
    :	tree(storage, alloc)
    {
    }

    /// Constructor initializing an empty B+ tree with a special key
    /// comparison object
    explicit inline btree_map
	(	storage_type& storage
	,	const key_compare &kcf
	,	const allocator_type &alloc = allocator_type())
    :	tree(storage, kcf, alloc)
    {
    }

    /// Constructor initializing a B+ tree with the range [first,last)
    template <class InputIterator>
    inline btree_map(InputIterator first, InputIterator last,
                     const allocator_type &alloc = allocator_type())
        : tree(first, last, alloc)
    {
    }

    /// Constructor initializing a B+ tree with the range [first,last) and a
    /// special key comparison object
    template <class InputIterator>
    inline btree_map(InputIterator first, InputIterator last, const key_compare &kcf,
                     const allocator_type &alloc = allocator_type())
        : tree(first, last, kcf, alloc)
    {
    }

    /// Frees up all used B+ tree memory pages
    inline ~btree_map()
    {
    }

    /// Fast swapping of two identical B+ tree objects.
    void swap(self& from)
    {
        std::swap(tree, from.tree);
    }

public:
    // *** Key and Value Comparison Function Objects

    /// Constant access to the key comparison object sorting the B+ tree
    inline key_compare key_comp() const
    {
        return tree.key_comp();
    }

    /// Constant access to a constructed value_type comparison object. required
    /// by the STL
    inline value_compare value_comp() const
    {
        return tree.value_comp();
    }

public:
    // *** Allocators

    /// Return the base node allocator provided during construction.
    allocator_type get_allocator() const
    {
        return tree.get_allocator();
    }

	/// Set the max use of the tree before using storage
	void set_max_use(ptrdiff_t max_use){
		tree.set_max_use(max_use);
	}


public:
    // *** Fast Destruction of the B+ Tree

    /// Frees all key/data pairs and all nodes of the tree
    void clear()
    {
        tree.clear();
    }

public:
    // *** STL Iterator Construction Functions

    /// Constructs a read/data-write iterator that points to the first slot in
    /// the first surface of the B+ tree.
    inline iterator begin()
    {
        return tree.begin();
    }

    /// Constructs a read/data-write iterator that points to the first invalid
    /// slot in the last surface of the B+ tree.
    inline iterator end()
    {
        return tree.end();
    }

    /// Constructs a read-only constant iterator that points to the first slot
    /// in the first surface of the B+ tree.
    inline const_iterator begin() const
    {
        return tree.begin();
    }

    /// Constructs a read-only constant iterator that points to the first
    /// invalid slot in the last surface of the B+ tree.
    inline const_iterator end() const
    {
        return tree.end();
    }

    /// Constructs a read/data-write reverse iterator that points to the first
    /// invalid slot in the last surface of the B+ tree. Uses STL magic.
    inline reverse_iterator rbegin()
    {
        return tree.rbegin();
    }

    /// Constructs a read/data-write reverse iterator that points to the first
    /// slot in the first surface of the B+ tree. Uses STL magic.
    inline reverse_iterator rend()
    {
        return tree.rend();
    }

    /// Constructs a read-only reverse iterator that points to the first
    /// invalid slot in the last surface of the B+ tree. Uses STL magic.
    inline const_reverse_iterator rbegin() const
    {
        return tree.rbegin();
    }

    /// Constructs a read-only reverse iterator that points to the first slot
    /// in the first surface of the B+ tree. Uses STL magic.
    inline const_reverse_iterator rend() const
    {
        return tree.rend();
    }

public:
    // *** Access Functions to the Item Count

    /// Return the number of key/data pairs in the B+ tree
    inline size_type size() const
    {
        return tree.size();
    }

    /// Returns true if there is at least one key/data pair in the B+ tree
    inline bool empty() const
    {
        return tree.empty();
    }

    /// Returns the largest possible size of the B+ Tree. This is just a
    /// function required by the STL standard, the B+ Tree can hold more items.
    inline size_type max_size() const
    {
        return tree.max_size();
    }

    /// Return a const reference to the current statistics.
    inline const tree_stats& get_stats() const
    {
        return tree.get_stats();
    }

public:
    // *** Standard Access Functions Querying the Tree by Descending to a surface

    /// Non-STL function checking whether a key is in the B+ tree. The same as
    /// (find(k) != end()) or (count() != 0).
    bool exists(const key_type &key) const
    {
        return tree.exists(key);
    }

    /// Tries to locate a key in the B+ tree and returns an iterator to the
    /// key/data slot if found. If unsuccessful it returns end().
    iterator find(const key_type &key)
    {
        return tree.find(key);
    }
    /// Tries to locate a key in the B+ tree and returns a pointer to the
    /// key/data slot if found. If unsuccessful it returns nullptr.

    const data_type* direct(const key_type &key) const
    {
        return tree.direct(key);
        
    }
    /// Tries to locate a key in the B+ tree and returns a pointer to the
    /// key/data slot if found. If unsuccessful it returns nullptr.

    data_type* direct(const key_type &key)
    {

        data_type* dp = tree.direct(key);
        if(dp!= nullptr)
            return dp;
        iterator i = find( key );
        return i != end() ? &(i.data()) : nullptr;
    }

    /// Tries to locate a key in the B+ tree and returns an constant iterator
    /// to the key/data slot if found. If unsuccessful it returns end().
    const_iterator find(const key_type &key) const
    {
        return tree.find(key);
    }

    /// Tries to locate a key in the B+ tree and returns the number of
    /// identical key entries found. Since this is a unique map, count()
    /// returns either 0 or 1.
    size_type count(const key_type &key) const
    {
        return tree.count(key);
    }

    /// Searches the B+ tree and returns an iterator to the first pair
    /// equal to or greater than key, or end() if all keys are smaller.
    iterator lower_bound(const key_type& key)
    {
        return tree.lower_bound(key);
    }

    /// Searches the B+ tree and returns a constant iterator to the
    /// first pair equal to or greater than key, or end() if all keys
    /// are smaller.
    const_iterator lower_bound(const key_type& key) const
    {
        return tree.lower_bound(key);
    }

    /// Searches the B+ tree and returns an iterator to the first pair
    /// greater than key, or end() if all keys are smaller or equal.
    iterator upper_bound(const key_type& key)
    {
        return tree.upper_bound(key);
    }

    /// Searches the B+ tree and returns a constant iterator to the
    /// first pair greater than key, or end() if all keys are smaller
    /// or equal.
    const_iterator upper_bound(const key_type& key) const
    {
        return tree.upper_bound(key);
    }

    /// Searches the B+ tree and returns both lower_bound() and upper_bound().
    inline std::pair<iterator, iterator> equal_range(const key_type& key)
    {
        return tree.equal_range(key);
    }

    /// Searches the B+ tree and returns both lower_bound() and upper_bound().
    inline std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const
    {
        return tree.equal_range(key);
    }

public:
    // *** B+ Tree Object Comparison Functions

    /// Equality relation of B+ trees of the same type. B+ trees of the same
    /// size and equal elements (both key and data) are considered
    /// equal.
    inline bool operator==(const self &other) const
    {
        return (tree == other.tree);
    }

    /// Inequality relation. Based on operator==.
    inline bool operator!=(const self &other) const
    {
        return (tree != other.tree);
    }

    /// Total ordering relation of B+ trees of the same type. It uses
    /// std::lexicographical_compare() for the actual comparison of elements.
    inline bool operator<(const self &other) const
    {
        return (tree < other.tree);
    }

    /// Greater relation. Based on operator<.
    inline bool operator>(const self &other) const
    {
        return (tree > other.tree);
    }

    /// Less-equal relation. Based on operator<.
    inline bool operator<=(const self &other) const
    {
        return (tree <= other.tree);
    }

    /// Greater-equal relation. Based on operator<.
    inline bool operator>=(const self &other) const
    {
        return (tree >= other.tree);
    }

public:
    /// *** Fast Copy: Assign Operator and Copy Constructors

    /// Assignment operator. All the key/data pairs are copied
    inline self& operator= (const self &other)
    {
        if (this != &other)
        {
            tree = other.tree;
        }
        return *this;
    }

    /// Copy constructor. The newly initialized B+ tree object will contain a
    /// copy of all key/data pairs.
    inline btree_map(const self &other)
        : tree(other.tree)
    {
    }

public:
    // *** Public Insertion Functions

    /// Attempt to insert a key/data pair into the B+ tree. Fails if the pair
    /// is already present.
    inline std::pair<iterator, bool> insert(const value_type& x)
    {
        return tree.insert2(x.first, x.second);
    }

    /// Attempt to insert a key/data pair into the B+ tree. Beware that if
    /// key_type == data_type, then the template iterator insert() is called
    /// instead. Fails if the inserted pair is already present.
    inline std::pair<iterator, bool> insert(const key_type& key, const data_type& data)
    {
        return tree.insert2(key, data);
    }

    /// Attempt to insert a key/data pair into the B+ tree. This function is the
    /// same as the other insert, however if key_type == data_type then the
    /// non-template function cannot be called. Fails if the inserted pair is
    /// already present.
    inline std::pair<iterator, bool> insert2(const key_type& key, const data_type& data)
    {
        return tree.insert2(key, data);
    }

    /// Attempt to insert a key/data pair into the B+ tree. The iterator hint
    /// is currently ignored by the B+ tree insertion routine.
    inline iterator insert(iterator hint, const value_type &x)
    {
        return tree.insert2(hint, x.first, x.second);
    }

    /// Attempt to insert a key/data pair into the B+ tree. The iterator hint is
    /// currently ignored by the B+ tree insertion routine.
    inline iterator insert2(iterator hint, const key_type& key, const data_type& data)
    {
        return tree.insert2(hint, key, data);
    }

    /// Returns a reference to the object that is associated with a particular
    /// key. If the map does not already contain such an object, operator[]
    /// inserts the default object data_type().
    inline data_type& operator[](const key_type& key)
    {



        iterator i = insert( value_type(key, data_type()) ).first;

        return i.data();
    }

	/// Returns a const reference to the object that is associated with a particular
    /// key. If the map does not already contain such an object, operator[] const
    /// returns the object at the end.
	inline const data_type& operator[](const key_type& key) const
    {
        const data_type* dp = tree.direct(key);
        if(dp!= nullptr)
            return *dp;
        const_iterator i = find( key );
        return i.data();
    }

    /// Attempt to insert the range [first,last) of value_type pairs into the B+
    /// tree. Each key/data pair is inserted individually.
    template <typename InputIterator>
    inline void insert(InputIterator first, InputIterator last)
    {
        return tree.insert(first, last);
    }

public:

    /// check the current use flag and flush buffers of required
    void check_use(){
        tree.check_low_memory_state();
    }
	/// reduce the tree memory use by storing pages to alternate storage

    void reduce_use(){
		tree.flush_buffers(true);
	}
	/// store written pages
	 void flush_buffers(){
		tree.flush_buffers(false);
	}
	// *** Public Erase Functions

    /// Erases the key/data pairs associated with the given key. For this
    /// unique-associative map there is no difference to erase().
    bool erase_one(const key_type &key)
    {
        return tree.erase_one(key);
    }

    /// Erases all the key/data pairs associated with the given key. This is
    /// implemented using erase_one().
    size_type erase(const key_type &key)
    {
        return tree.erase(key);
    }

    /// Erase the key/data pair referenced by the iterator.
    void erase(iterator iter)
    {
        return tree.erase(iter);
    }

#ifdef BTREE_TODO
    /// Erase all key/data pairs in the range [first,last). This function is
    /// currently not implemented by the B+ Tree.
    void erase(iterator /* first */, iterator /* last */)
    {
        abort();
    }
#endif

#ifdef BTREE_DEBUG
public:
    // *** Debug Printing

    /// Print out the B+ tree structure with keys onto the given ostream. This function
    /// requires that the header is compiled with BTREE_DEBUG and that key_type
    /// is printable via std::ostream.
    void print(std::ostream &os) const
    {
        tree.print(os);
    }

    /// Print out only the leaves via the double linked list.
    void print_leaves(std::ostream &os) const
    {
        tree.print_leaves(os);
    }
#endif

public:
    // *** Verification of B+ Tree Invariants

    /// Run a thorough verification of all B+ tree invariants. The program
    /// aborts via BTREE_ASSERT() if something is wrong.
    void verify() const
    {
        tree.verify();
    }

public:
	/// shared pages

	void share(std::string name){

	}


	stx::storage::stream_address get_root_address() const {
		return tree.get_root_address();
	}
    /// encodes and writes all active nodes which are still modified

    void flush()    {
        tree.flush();

    }

	/// loads the tree from the specified root address - all previous changes are lost

	void restore(stx::storage::stream_address r){

		tree.restore(r);

	}

	/// when a tree becomes stale

	void reload()
	{
		tree.reload();

	}

	/// restore an iterator from an initializer pair
	void restore_iterator(iterator& out, const typename iterator::initializer_pair& init){
		out.from_initializer(tree, init);
	}

};

} // namespace stx

#endif // _STX_BTREE_MAP_H_
