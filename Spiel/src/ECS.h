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
using entity_handle = uint32_t;
struct entity_id {
	entity_id(entity_id_type id = 0) : id{ id } {}
	entity_id_type id;
};

using storage_t = int;

constexpr storage_t direct_indexing = 0;
constexpr storage_t hashing = 1;
constexpr storage_t lookup_table = 2;

template<typename CompType, storage_t storageType>
class ComponentStorage {
public:
	inline void insert(entity_handle entity, CompType const& comp);
	inline void remove(entity_handle entity);
	inline bool contains(entity_handle entity) const;
	inline CompType& get(entity_handle entity);
	inline CompType& operator[](entity_handle ent);
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
		entity_handle id();
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
	using storage_t = robin_hood::unordered_map<uint32_t, CompType>;

	inline void insert(entity_handle entity, CompType const& comp) {
		if (entity >= containsVec.size()) containsVec.resize(entity + 1, false);
		containsVec[entity] = true;

		storage.insert({ entity, comp });
	}
	inline void remove(entity_handle entity) {
		if (contains(entity)) {
			containsVec[entity] = false;
			auto res = storage.find(entity);
			if (res != storage.end()) {
				storage.erase(res);
			}
		}
	}
	inline bool contains(entity_handle entity) const {
		if (entity < containsVec.size()) {
			return containsVec[entity];
		}
		else {
			return false;
		}
	}
	inline CompType& get(entity_handle entity) {
		assert(contains(entity));
		return storage[entity];
	}
	inline CompType& operator[](entity_handle entity) {
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
		entity_handle handle() { return iter->first; }
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
	using storage_t = std::vector<CompType>;

	inline void insert(entity_handle entity, CompType const& comp) {
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
	inline void remove(entity_handle entity) {
		if (contains(entity)) {
			containsVec[entity] = false;
		}
	}
	inline bool contains(entity_handle entity) const {
		if (entity < containsVec.size()) {
			return containsVec[entity];
		}
		else {
			return false;
		}
	}
	inline CompType& get(entity_handle entity) {
		assert(contains(entity));
		return storage[entity];
	}
	inline CompType& operator[](entity_handle entity) {
		assert(contains(entity));
		return storage[entity].second;
	}
	inline size_t size() const { return storage.size(); }
	class iterator {
	public:
		typedef iterator self_type;
		typedef CompType value_type;
		typedef CompType& reference;
		typedef CompType* pointer;
		typedef std::forward_iterator_tag iterator_category;
		iterator(entity_handle entity_, storage_t& storage_) : entity{ entity_ }, storage{ storage_ } {}
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
		entity_handle handle() {
			assert(entity < storage.size());
			return entity;
		}
	private:
		entity_handle entity;
		storage_t& storage;
	};
	inline iterator begin() {
		entity_handle id = 0;
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
	using storage_t = std::vector<CompType>;

	inline void optimiseMemoryLayout() {
		std::vector<entity_handle> storedIDs;
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
		freeStorageSlots = std::queue<entity_handle>();
	}

	inline void insert(entity_handle entity, CompType const& comp) {
		assert(!contains(entity));
		if (entity >= indexTable.size()) {
			indexTable.resize(entity + 1, 0xFFFFFFFF);
		}

		if (freeStorageSlots.empty()) {
			storage.emplace_back(comp);
			indexTable[entity] = storage.size() - 1;
		}
		else {
			entity_handle freeSlotIndex = freeStorageSlots.front();
			freeStorageSlots.pop();
			indexTable[entity] = freeSlotIndex;
			storage[indexTable[entity]] = comp;
		}
	}

	inline void remove(entity_handle entity) {
		assert(contains(entity));
		freeStorageSlots.push(indexTable[entity]);
		indexTable[entity] = 0xFFFFFFFF;
	}

	inline bool contains(entity_handle entity) const {
		assert(entity != 0);
		if (entity >= indexTable.size()) return false;
		else return indexTable[entity] != 0xFFFFFFFF;
	}

	inline CompType& get(entity_handle entity) {
		assert(contains(entity));
		return storage[indexTable[entity]];
	}
	inline CompType& operator[](entity_handle entity) {
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
		iterator(entity_handle entity_, storage_t& storage_, std::vector<entity_handle>& indexTable) : entity{ entity_ }, storage{ storage_ }, indexTable{ indexTable } {}
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
		entity_handle handle() {
			assert(entity < indexTable.size());
			return entity;
		}
	private:
		entity_handle entity;
		storage_t& storage;
		std::vector<entity_handle>& indexTable;
	};
private:
	std::vector<entity_handle> indexTable;
	storage_t storage;
	std::queue<entity_handle> freeStorageSlots;
};