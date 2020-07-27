#pragma once
#include <utility> 
#include <functional> // std::invoke

#include <cassert>
#include <iterator>
#include <variant>
#include <tuple>

#include <vector>

#include "robin_hood.h"

#include "EntityTypes.hpp"
#define DEBUG_COMPONENT_STORAGE


struct CompData {

};

using storage_t = int;

constexpr storage_t direct_indexing = 0;
constexpr storage_t hashing = 1;
constexpr storage_t lookup_table = 2;
constexpr storage_t sorted_lookup_table = 3;
constexpr storage_t sparse_set = 4;
constexpr storage_t shit = 5;

template<typename CompType, storage_t storageType>
class ComponentStorage {
public:
	using Component = CompType;
	inline void updateMaxEntNum(size_t newEntNum);
	inline size_t memoryConsumtion();
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

	inline size_t memoryConsumtion() {
		return storage.size();
	}

	inline void updateMaxEntNum(size_t newEntNum) {
		if (containsVec.size() < newEntNum) {
			containsVec.resize(newEntNum, false);
		}
	}

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
#ifdef DEBUG_COMPONENT_STORAGE
		return containsVec.at(entity);
#else
		return containsVec[entity];
#endif
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

	inline size_t memoryConsumtion() {
		return storage.capacity() * sizeof(CompType);
	}

	inline void updateMaxEntNum(size_t newEntNum) {
		if (containsVec.size() < newEntNum) {
			containsVec.resize(newEntNum, false);
		}
	}

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
#ifdef DEBUG_COMPONENT_STORAGE
		return containsVec.at(entity);
#else
return containsVec[entity];
#endif
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

	template<typename CompType>
	class iterator {
	public:
		typedef iterator self_type;
		typedef entity_index_type value_type;
		typedef entity_index_type& reference;
		typedef entity_index_type* pointer;
		typedef std::forward_iterator_tag iterator_category;
		iterator(entity_index_type entity_, ComponentStorage<CompType, direct_indexing>& compStore) : entity{ entity_ }, compStore{ compStore } {}
		self_type operator++(int dummy) {
			assert(entity < compStore.storage.size());
			++entity;
			while (entity < compStore.storage.size() && !compStore.containsVec.at(entity)) ++entity; //skip non valid entries
			return *this;
		}
		self_type operator++() {
			self_type me = *this;
			operator++(0);
			return me;
		}
		reference operator*() {
			assert(entity < compStore.storage.size() && compStore.containsVec.at(entity));
			return compStore.storage[entity];
		}
		pointer operator->() {
			assert(entity < compStore.storage.size());
			return &compStore.storage[entity];
		}
		bool operator==(self_type const& rhs) {
			return entity == rhs.entity;
		}
		bool operator!=(self_type const& rhs) {
			return entity != rhs.entity;
		}
		entity_index_type data() {
			assert(entity < storage.size());
			return compStore.storage[entity];
		}
	private:
		entity_index_type entity; 
		ComponentStorage<CompType, direct_indexing>& compStore;
	};
	inline iterator<CompType> begin() {
		entity_index_type entity = 0;
		while (!contains(entity)) ++entity;
		return iterator<CompType>(entity, *this);
	}
	inline iterator<CompType> end() { return iterator<CompType>(storage.size(), *this); }
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

	inline size_t memoryConsumtion() {
		return indexTable.capacity() * sizeof(uint32_t) + storage.capacity() * sizeof(CompType);
	}


	inline void updateMaxEntNum(size_t newEntNum) {
		if (indexTable.size() < newEntNum) {
			indexTable.resize(newEntNum, 0xFFFFFFFF);
		}
	}

	inline void optimiseMemoryLayout() {
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
		if (contains(entity)) {
			freeStorageSlots.push(indexTable[entity]);
			indexTable[entity] = 0xFFFFFFFF;
		}
	}

	inline bool contains(entity_index_type entity) const {
#ifdef DEBUG_COMPONENT_STORAGE
		assert(entity != 0);
		return indexTable.at(entity) != 0xFFFFFFFF;
#else
		return indexTable[entity] != 0xFFFFFFFF;
#endif
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


template<typename CompType>
class ComponentStorage<CompType, sparse_set> {
public:

	inline size_t memoryConsumtion() {
		return sparseTable.capacity() * sizeof(entity_index_type) + denseTable.capacity() * sizeof(entity_index_type); + storage.capacity() * sizeof(CompType);
	}

	inline void updateMaxEntNum(size_t newEntNum) {
		if (sparseTable.size() < newEntNum) {
			sparseTable.resize(newEntNum, 0xFFFFFFFF);
		}
	}

	inline void insert(entity_index_type entity, CompType const& comp) {
		assert(!contains(entity));
		assert(entity < sparseTable.size());
		
		denseTable.push_back(entity);
		storage.push_back(comp);
		sparseTable.at(entity) = denseTable.size() - 1;

		assert(storage.size() == denseTable.size());
	}

	inline void remove(entity_index_type entity) {
		if (contains(entity)) {
			if (entity == denseTable.at(denseTable.size() - 1)) {
				sparseTable.at(entity) = 0xFFFFFFFF;
				denseTable.pop_back();
				storage.pop_back();
			}
			else {
				auto slot = sparseTable.at(entity);
				sparseTable.at(entity) = 0xFFFFFFFF;
				auto lastEnt = denseTable.at(denseTable.size() - 1);
				denseTable.pop_back();
				sparseTable.at(lastEnt) = slot;
				denseTable.at(slot) = lastEnt;
				storage.at(slot) = storage.at(storage.size() - 1);
				storage.pop_back();
			}

			assert(storage.size() == denseTable.size());
		}
	}

	inline bool contains(entity_index_type entity) const {
		return sparseTable.at(entity) != 0xFFFFFFFF;
	}

	inline CompType& get(entity_index_type entity) {
		return storage[sparseTable[entity]];
	}
	inline CompType& operator[](entity_index_type entity) {
		return get(entity);
	}
	inline size_t size() const {
		return denseTable.size();
	}
	template<typename CompType>
	class iterator {
	public:
		typedef iterator self_type;
		typedef entity_index_type value_type;
		typedef entity_index_type& reference;
		typedef entity_index_type* pointer;
		typedef std::forward_iterator_tag iterator_category;
		iterator(entity_index_type denseTableIndex, ComponentStorage<CompType, sparse_set>& compStore) : denseTableIndex{ denseTableIndex }, compStore{ compStore } {}
		self_type operator++(int dummy) {
			assert(denseTableIndex < compStore.denseTable.size());
			++denseTableIndex;
			return *this;
		}
		self_type operator++() {
			self_type me = *this;
			operator++(0);
			return me;
		}
		reference operator*() {
			return compStore.denseTable.at(denseTableIndex);
		}
		pointer operator->() {
			return &compStore.denseTable.at(denseTableIndex);
		}
		bool operator==(self_type const& rhs) {
			return denseTableIndex == denseTableIndex;
		}
		bool operator!=(self_type const& rhs) {
			return denseTableIndex != rhs.denseTableIndex;
		}
		CompType data() {
			return compStore.storage.at(denseTableIndex);
		}
	private:
		entity_index_type denseTableIndex; 
		ComponentStorage<CompType, sparse_set>& compStore;
	};
	iterator<CompType> begin() { return iterator<CompType>(0, *this); }
	iterator<CompType> end() { return iterator<CompType>(denseTable.size(), *this); }
private:
	std::vector<entity_index_type> sparseTable;
	std::vector<entity_index_type> denseTable;
	std::vector<CompType> storage;
};

template<typename CompType>
class ComponentStorage<CompType, sorted_lookup_table> {
public:
	using Component = CompType;
	using storage_t = std::vector<CompType>;

	inline size_t memoryConsumtion() {
		return indexTable.capacity() * sizeof(uint32_t) + storage.capacity() * sizeof(CompType);
	}

	inline void updateMaxEntNum(size_t newEntNum) {
		if (indexTable.size() < newEntNum) {
			indexTable.resize(newEntNum, 0xFFFFFFFF);
		}
	}

	inline void insert(entity_index_type entity, CompType const& comp) {
		// find next bugger entity index
		entity_index_type nextBiggerEntityIndex = entity + 1;
		while (nextBiggerEntityIndex < indexTable.size() && !contains(nextBiggerEntityIndex)) nextBiggerEntityIndex++;

		if (nextBiggerEntityIndex == indexTable.size()) {
			// there is no next bigger entity.

			storage.push_back(comp);
			indexTable[entity] = storage.size() - 1;
		}
		else {
			// there is a bigger entity.

			int nseiStorageIndex = indexTable[nextBiggerEntityIndex];
			auto insertionPlace = storage.begin() + nseiStorageIndex;
			// insert new storage at that place
			storage.insert(insertionPlace, comp);
			// set entities index in table to new entry
			indexTable[entity] = nseiStorageIndex;

			// move indices in index tables one to the right
			for (auto ent = nextBiggerEntityIndex; ent < indexTable.size(); ent++) {
				if (contains(ent)) {
					indexTable[ent] += 1;
				}
			}
		}
	}

	inline void remove(entity_index_type entity) {
		if (contains(entity)) {
			// erase entity place
			int index = indexTable[entity];
			auto erasePlace = storage.begin() + index;
			storage.erase(erasePlace);
			indexTable[entity] = 0xFFFFFFFF;

			// move indices of indexTalbe to the left:
			for (auto ent = entity + 1; entity < indexTable.size(); entity++) {
				if (contains(entity)) {
					indexTable[entity] -= 1;
				}
			}
		}
	}

	inline bool contains(entity_index_type entity) const {
#ifdef DEBUG_COMPONENT_STORAGE
		return indexTable.at(entity) != 0xFFFFFFFF;
#else
		return indexTable[entity] != 0xFFFFFFFF;
#endif
	}

	inline CompType& get(entity_index_type entity) {
		assert(contains(entity));
		return storage[indexTable[entity]];
	}
	inline CompType& operator[](entity_index_type entity) {
		return get(entity);
	}
	inline size_t size() const {
		return storage.size();
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
};



template<typename CompType>
class ComponentStorage<CompType, shit> {
public:

	inline size_t memoryConsumtion() {
		return 0;
	}

	inline void updateMaxEntNum(size_t newEntNum) {
		if (components.size() < newEntNum) {
			components.resize(newEntNum, nullptr);
		}
	}

	inline void insert(entity_index_type entity, CompType const& comp) {
		if (!contains(entity)) {
			components[entity] = new CompType(comp);
		}
	}

	inline void remove(entity_index_type entity) {
		if (contains(entity)) {
			delete components[entity];
			components[entity] = nullptr;
		}
	}

	inline bool contains(entity_index_type entity) const {
#ifdef DEBUG_COMPONENT_STORAGE
		return components.at(entity) != nullptr;
#else
		return components[entity] != nullptr;
#endif
	}

	inline CompType& get(entity_index_type entity) {
		return operator[](entity);
	}
	inline CompType& operator[](entity_index_type entity) {
		return *(components[entity]);
	}
	inline size_t size() const {
		return components.size();
	}
private:
	std::vector<CompType*> components;
};



template< size_t I, typename T, typename Tuple_t>
constexpr size_t index_in_storagetuple_fn() {
	static_assert(I < std::tuple_size<Tuple_t>::value, "The element is not in the tuple");

	typedef typename std::tuple_element<I, Tuple_t>::type el;
	if constexpr (std::is_same<ComponentStorage<T, hashing>, el>::value
		|| std::is_same<ComponentStorage<T, direct_indexing>, el>::value
		|| std::is_same<ComponentStorage<T, lookup_table>, el>::value
		|| std::is_same<ComponentStorage<T, sorted_lookup_table>, el>::value
		|| std::is_same<ComponentStorage<T, shit>, el>::value
		|| std::is_same<ComponentStorage<T, sparse_set>, el>::value) {
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