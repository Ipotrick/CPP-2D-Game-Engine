#pragma once
#include <utility> 
#include <functional> // std::invoke

#include <cassert>
#include <iterator>
#include <variant>
#include <tuple>

#include <vector>

#include "robin_hood.h"

struct CompData {

};

using entity_id_type = uint32_t;
using entity_index_type = uint32_t;
struct entity_id {
	explicit entity_id(entity_id_type id = 0, uint32_t version = 0) : id{ id }, version{ version } {}
	void operator=(entity_id rhs) {
		this->id = rhs.id;
		this->version = rhs.version;
	}
	entity_id_type& operator*() { return id; }
	entity_id_type id;
	uint32_t version;
};

using storage_t = int;

constexpr storage_t direct_indexing = 0;
constexpr storage_t hashing = 1;
constexpr storage_t lookup_table = 2;

template<typename CompType, storage_t storageType>
class ComponentStorage {
public:
	using Component = CompType;
	inline void insert(entity_index_type entity, CompType const& comp);
	inline void remove(entity_index_type entity);
	inline bool contains(entity_index_type entity) const;
	inline CompType& get(entity_index_type entity);
	inline CompType& operator[](entity_index_type ent);
	inline size_t size() const;
	class iterator {
	public:
		typedef iterator self_type;
		typedef CompType value_type;
		typedef CompType& reference;
		typedef CompType* pointer;
		typedef std::forward_iterator_tag iterator_category;
		self_type operator++();
		self_type operator++(int junk);
		reference operator*();
		pointer operator->();
		bool operator==(const self_type& rhs);
		bool operator!=(const self_type& rhs);
		entity_index_type id();
	};
	inline iterator begin();
	inline iterator end();
};

/*
	pros:
		-low memory usage
	cons: 
		-fast access time
		-unordered, accessing OTHER components in a loop is not that cache friendly
		-no interloop comp registration
		-no interloop comp deregistration
		violating these limitations will mass up the iterators
	memory alignment:
		random
		packed (1.3)
	memory usage:
		n = count(entities)
		nMax = (current maximum of the entitiy List)
		m(n, nMax) = nMax + n * sizeof(CompType) * 8 * 1.3;
*/
template<typename CompType>
class ComponentStorage<CompType, hashing> {
public:
	using Component = CompType;
	using storage_t = robin_hood::unordered_map<uint32_t, CompType>;

	inline void insert(entity_index_type entity, CompType const& comp) {
		assert(!contains(entity));
		if (entity >= containsVec.size()) containsVec.resize(entity + 1, false);
		containsVec[entity] = true;

		storage[entity] = comp;
	}
	inline void remove(entity_index_type entity) {
		if (contains(entity)) {
			containsVec[entity] = false;
			auto res = storage.find(entity);
			if (res != storage.end()) {
				storage.erase(res);
			}
		}
	}
	inline bool contains(entity_index_type entity) const {
		if (entity < containsVec.size()) {
			return containsVec[entity];
		}
		else {
			return false;
		}
	}
	inline CompType& get(entity_index_type entity) {
		assert(contains(entity));
		return storage[entity];
	}
	inline CompType& operator[](entity_index_type entity) {
		assert(contains(entity));
		return storage[entity];
	}
	inline size_t size() const { return storage.size(); }
	class iterator {
	public:
		typedef iterator self_type;
		typedef CompType value_type;
		typedef CompType& reference;
		typedef CompType* pointer;
		typedef std::forward_iterator_tag iterator_category;
		iterator(typename storage_t::iterator storIter, storage_t& stor_) : iter{ storIter }, storage{ stor_ } {}
		self_type operator++(int junk) {
			assert(iter != storage.end());
			++iter;
			return *this;
		}
		self_type operator++() {
			self_type me = *this;
			operator++(0);
			return me;
		}
		reference operator*() {
			assert(iter != storage.end());
			return iter->second;
		}
		pointer operator->() {
			assert(iter != storage.end());
			return &(iter->second);
		}
		bool operator==(const self_type& rhs) {
			return iter == rhs.iter;
		}
		bool operator!=(const self_type& rhs) {
			return iter != rhs.iter;
		}
		entity_index_type handle() { return iter->first; }
	private:
		typename storage_t::iterator iter;
		storage_t& storage;
	};
	inline iterator begin() { return iterator(storage.begin(), storage); }
	inline iterator end() { return iterator(storage.end(), storage); }
private:
	storage_t storage;
	std::vector<bool> containsVec;
};

/*
	pros:
		instant access
		instant deregistration
		instant registration
		linear iteration, super cache friendly
		iterators NEVER get invalidated
	cons:
		very high memory usage
	memory alignment:
		linear
		sparse (n / nMax)
	memory usage:
		n = count(entities)
		nMax = (current maximum of the entitiy List)
		m(n, nMax) = nMax + nMax * sizeof(CompType) * 8;

*/
template<typename CompType>
class ComponentStorage<CompType, direct_indexing>{
public:
	using Component = CompType;
	using storage_t = std::vector<CompType>;

	inline void insert(entity_index_type entity, CompType const& comp) {
		assert(!contains(entity));
		if (entity >= containsVec.size()) containsVec.resize(entity + 1, false);
		containsVec[entity] = true;
		if (entity < storage.size()) {
			storage[entity] = comp;
		}
		else if (entity == storage.size()) {
			storage.push_back(comp);
		}
		else if (entity > storage.size()) {
			storage.resize(entity+1, CompType());
			storage[entity] = comp;
		}
	}
	inline void remove(entity_index_type entity) {
		if (contains(entity)) {
			containsVec[entity] = false;
		}
	}
	inline bool contains(entity_index_type entity) const {
		if (entity < containsVec.size()) {
			return containsVec[entity];
		}
		else {
			return false;
		}
	}
	inline CompType& get(entity_index_type entity) {
		assert(contains(entity));
		return storage[entity];
	}
	inline CompType& operator[](entity_index_type entity) {
		assert(contains(entity));
		return storage[entity];
	}
	inline size_t size() const { return storage.size(); }
	class iterator {
	public:
		typedef iterator self_type;
		typedef CompType value_type;
		typedef CompType& reference;
		typedef CompType* pointer;
		typedef std::forward_iterator_tag iterator_category;
		iterator(entity_index_type entity_, storage_t& storage_) : entity{ entity_ }, storage{ storage_ } {}
		self_type operator++(int dummy) {
			assert(entity < storage.size());
			++entity;
			while (!contains(entity)) ++entity; //skip non valid entries
			return *this;
		}
		self_type operator++() {
			self_type me = *this;
			operator++(0);
			return me;
		}
		reference operator*() {
			assert(entity < storage.size());
			return storage[entity];
		}
		pointer operator->() {
			assert(entity < storage.size());
			return &storage[entity];
		}
		bool operator==(self_type const& rhs) {
			return entity == entity;
		}
		bool operator!=(self_type const& rhs) {
			return entity != rhs.entity;
		}
		entity_index_type handle() {
			assert(entity < storage.size());
			return entity;
		}
	private:
		entity_index_type entity;
		storage_t& storage;
	};
	inline iterator begin() {
		entity_index_type id = 0;
		while (!contains(id)) ++id;
		return iterator(id, storage);
	}
	inline iterator end() { return iterator(storage.size(), storage); }
private:
	storage_t storage;
	std::vector<bool> containsVec;
};

/*
	pros:
		instant access
		instant deregistration
		very fast registration
		iterators NEVER get invalidated
		memory usage on large objects that appear often is very good
	cons:
		random iteration BUT data is tightly packed
		index table memory overhead ( 4 bytes * MaxEntID )
	memory alignment:
		random (can be optimised by call to linear)
		packed (1)
	memory usage:
		n = count(entities)
		nMax = (current maximum of the entitiy List)
		m(n, nMax) = nMax * 32 + n * sizeof(CompType) * 8

*/
template<typename CompType>
class ComponentStorage<CompType, lookup_table> {
public:
	using Component = CompType;
	using storage_t = std::vector<CompType>;

	inline void optimiseMemoryLayout() {
		std::vector<entity_index_type> storedIDs;
		storedIDs.reserve(storage.size());
		for (auto id : *this) {
			storedIDs.push_back(id);
		}
		// rebuild storage
		auto oldStorage = storage;
		storage.clear();
		for (auto id : storedIDs) {
			storage.push_back(oldStorage[id]);
		}
		// rebuild indexTable
		int n = 0;
		for (auto id : storedIDs) {
			indexTable[id] = n;
		}
		storage.shrink_to_fit();
		freeStorageSlots = std::queue<entity_index_type>();
	}

	inline void insert(entity_index_type entity, CompType const& comp) {
		assert(!contains(entity));
		if (entity >= indexTable.size()) {
			indexTable.resize(entity + 1, 0xFFFFFFFF);
		}

		if (freeStorageSlots.empty()) {
			storage.emplace_back(comp);
			indexTable[entity] = storage.size() - 1;
		}
		else {
			entity_index_type freeSlotIndex = freeStorageSlots.front();
			freeStorageSlots.pop();
			indexTable[entity] = freeSlotIndex;
			storage[indexTable[entity]] = comp;
		}
	}

	inline void remove(entity_index_type entity) {
		assert(contains(entity));
		freeStorageSlots.push(indexTable[entity]);
		indexTable[entity] = 0xFFFFFFFF;
	}

	inline bool contains(entity_index_type entity) const {
		assert(entity != 0);
		if (entity >= indexTable.size()) return false;
		else return indexTable[entity] != 0xFFFFFFFF;
	}

	inline CompType& get(entity_index_type entity) {
		assert(contains(entity));
		return storage[indexTable[entity]];
	}
	inline CompType& operator[](entity_index_type entity) {
		return get(entity);
	}
	inline size_t size() const { 
		return storage.size() - freeStorageSlots.size();
	}
	class iterator {
	public:
		typedef iterator self_type;
		typedef CompType value_type;
		typedef CompType& reference;
		typedef CompType* pointer;
		typedef std::forward_iterator_tag iterator_category;
		iterator(entity_index_type entity_, storage_t& storage_, std::vector<entity_index_type>& indexTable) : entity{ entity_ }, storage{ storage_ }, indexTable{ indexTable } {}
		self_type operator++(int dummy) {
			assert(entity < indexTable.size());
			++entity;
			while (entity < indexTable.size() && indexTable[entity] == 0xFFFFFFFF) ++entity; //skip non valid entries
			return *this;
		}
		self_type operator++() {
			self_type me = *this;
			operator++(0);
			return me;
		}
		reference operator*() {
			assert(entity < indexTable.size());
			return storage[indexTable[entity]];
		}
		pointer operator->() {
			assert(entity < indexTable.size());
			return &storage[indexTable[entity]];
		}
		bool operator==(self_type const& rhs) {
			return entity == entity;
		}
		bool operator!=(self_type const& rhs) {
			return entity != rhs.entity;
		}
		entity_index_type handle() {
			assert(entity < indexTable.size());
			return entity;
		}
	private:
		entity_index_type entity;
		storage_t& storage;
		std::vector<entity_index_type>& indexTable;
	};
private:
	std::vector<entity_index_type> indexTable;
	storage_t storage;
	std::queue<entity_index_type> freeStorageSlots;
};

template< size_t I, typename T, typename Tuple_t>
constexpr size_t index_in_storagetuple_fn() {
	static_assert(I < std::tuple_size<Tuple_t>::value, "The element is not in the tuple");

	typedef typename std::tuple_element<I, Tuple_t>::type el;
	if constexpr (std::is_same<ComponentStorage<T, hashing>, el>::value
		|| std::is_same<ComponentStorage<T, direct_indexing>, el>::value
		|| std::is_same<ComponentStorage<T, lookup_table>, el>::value) {
		return I;
	}
	else {
		return index_in_storagetuple_fn<I + 1, T, Tuple_t>();
	}
}

template<typename T, typename Tuple_t>
struct index_in_storagetuple {
	static constexpr size_t value = index_in_storagetuple_fn<0, T, Tuple_t>();
};