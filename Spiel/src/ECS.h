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

using ent_id_t = uint32_t;

using storage_t = int;

constexpr storage_t direct_indexing = 0;
constexpr storage_t hashing = 1;
constexpr storage_t lookup_table = 2;

template<typename CompType, storage_t storageType>
class ComponentStorage {
public:
	inline void insert(ent_id_t entity, CompType const& comp);
	inline void remove(ent_id_t entity);
	inline bool contains(ent_id_t entity) const;
	inline CompType& get(ent_id_t entity);
	inline CompType& operator[](ent_id_t ent);
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
		ent_id_t id();
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
*/
template<typename CompType>
class ComponentStorage<CompType, hashing> {
public:
	using storage_t = robin_hood::unordered_map<uint32_t, CompType>;

	inline void insert(ent_id_t entity, CompType const& comp) {
		storage.insert({ entity, comp });
	}
	inline void remove(ent_id_t entity) {
		auto res = storage.find(entity);
		if (res != storage.end()) {
			storage.erase(res);
		}
	}
	inline bool contains(ent_id_t entity) const {
		return storage.contains(entity);
	}
	inline CompType& get(ent_id_t entity) {
		assert(contains(entity));
		return storage[entity];
	}
	inline CompType& operator[](ent_id_t entity) {
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
		ent_id_t id() { return iter->first; }
	private:
		typename storage_t::iterator iter;
		storage_t& storage;
	};
	inline iterator begin() { return iterator(storage.begin(), storage); }
	inline iterator end() { return iterator(storage.end(), storage); }
private:
	storage_t storage;
};

/*
	pros:
		instant access
		instant deregistration
		very fast registration
		linear iteration, super cache friendly
		iterators NEVER get invalidated
	cons:
		very high memory usage

*/
template<typename CompType>
class ComponentStorage<CompType, direct_indexing>{
public:
	using storage_t = std::vector<std::pair<bool, CompType>>;

	inline void insert(ent_id_t entity, CompType const& comp) {
		if (entity < storage.size()) {
			assert(storage[entity].first != true);	//cant register a registered entity
			storage[entity].first = true;
			storage[entity].second = comp;
		}
		else if (entity == storage.size()) {
			storage.push_back({ true, comp });
		}
		else if (entity > storage.size()) {
			storage.resize(entity, std::pair<bool, CompType>(false, CompType()));
			storage.push_back({ true, comp });
		}
	}
	inline void remove(ent_id_t entity) {
		if (entity < storage.size()) {
			storage[entity].first = false;
		}
	}
	inline bool contains(ent_id_t entity) const {
		if (entity < storage.size()) {
			return storage[entity].first;
		}
		else {
			return false;
		}
	}
	inline CompType& get(ent_id_t entity) {
		assert(contains(entity));
		return storage[entity].second;
	}
	inline CompType& operator[](ent_id_t entity) {
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
		iterator(ent_id_t entity_, storage_t& storage_) : entity{ entity_ }, storage{ storage_ } {}
		self_type operator++(int dummy) {
			assert(entity < storage.size());
			++entity;
			while (entity < storage.size() && storage[entity].first == false) ++entity; //skip non valid entries
			return *this;
		}
		self_type operator++() {
			self_type me = *this;
			operator++(0);
			return me;
		}
		reference operator*() {
			assert(entity < storage.size());
			return storage[entity].second;
		}
		pointer operator->() {
			assert(entity < storage.size());
			return &storage[entity].second;
		}
		bool operator==(self_type const& rhs) {
			return entity == entity;
		}
		bool operator!=(self_type const& rhs) {
			return entity != rhs.entity;
		}
		ent_id_t id() {
			assert(entity < storage.size());
			return entity;
		}
	private:
		ent_id_t entity;
		storage_t& storage;
	};
	inline iterator begin() {
		ent_id_t id = 0;
		while (id < storage.size() && storage[id].first == false) ++id;
		return iterator(id, storage);
	}
	inline iterator end() { return iterator(storage.size(), storage); }
private:
	storage_t storage;
};

/*
	pros:
		instant access
		instant deregistration
		very fast registration
		random iteration BUT data is tightly packed
		iterators NEVER get invalidated
		memory usage on large objects that appear often is very good
	cons:
		index table memory overhead ( 4 bytes * MaxEntID )

*/
template<typename CompType>
class ComponentStorage<CompType, lookup_table> {
public:
	using storage_t = std::vector<CompType>;

	inline void optimiseMemoryLayout() {
		std::vector<ent_id_t> storedIDs;
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
		freeStorageSlots = std::queue<ent_id_t>();
	}

	inline void insert(ent_id_t entity, CompType const& comp) {
		assert(!contains(entity));
		if (entity >= indexTable.size()) {
			indexTable.resize(entity + 1, 0xFFFFFFFF);
		}

		if (freeStorageSlots.empty()) {
			storage.emplace_back(comp);
			indexTable[entity] = storage.size() - 1;
		}
		else {
			ent_id_t freeSlotIndex = freeStorageSlots.front();
			freeStorageSlots.pop();
			indexTable[entity] = freeSlotIndex;
			storage[indexTable[entity]] = comp;
		}
	}

	inline void remove(ent_id_t entity) {
		assert(contains(entity));
		freeStorageSlots.push(indexTable[entity]);
		indexTable[entity] = 0xFFFFFFFF;
	}

	inline bool contains(ent_id_t entity) const {
		assert(entity != 0);
		if (entity >= indexTable.size()) return false;
		else return indexTable[entity] != 0xFFFFFFFF;
	}

	inline CompType& get(ent_id_t entity) {
		assert(contains(entity));
		return storage[indexTable[entity]];
	}
	inline CompType& operator[](ent_id_t entity) {
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
		iterator(ent_id_t entity_, storage_t& storage_, std::vector<ent_id_t>& indexTable) : entity{ entity_ }, storage{ storage_ }, indexTable{ indexTable } {}
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
		ent_id_t id() {
			assert(entity < indexTable.size());
			return entity;
		}
	private:
		ent_id_t entity;
		storage_t& storage;
		std::vector<ent_id_t>& indexTable;
	};
private:
	std::vector<ent_id_t> indexTable;
	storage_t storage;
	std::queue<ent_id_t> freeStorageSlots;
};