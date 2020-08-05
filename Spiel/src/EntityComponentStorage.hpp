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
constexpr storage_t sparse_indexing = 1;
constexpr storage_t sorted_lookup_table = 2;
constexpr storage_t sparse_set = 3;

template<typename CompType, storage_t storageType>
class ComponentStorage {
public:
	using Component = CompType;
	inline void updateMaxEntNum(size_t newEntNum);
	inline size_t memoryConsumtion();
	inline void insert(Entity entity, CompType const& comp);
	inline void remove(Entity entity);
	inline bool contains(Entity entity) const;
	inline CompType& get(Entity entity);
	inline CompType& operator[](Entity ent);
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
		Entity id();
	};
	inline iterator begin();
	inline iterator end();
};

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

	inline void insert(Entity entity, CompType const& comp) {
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
	inline void remove(Entity entity) {
		if (contains(entity)) {
			containsVec[entity] = false;
		}
	}
	inline bool contains(Entity entity) const {
		return containsVec[entity];
	}
	inline CompType& get(Entity entity) {
		assert(contains(entity));
		return storage[entity];
	}
	inline CompType& operator[](Entity entity) {
		assert(contains(entity));
		return storage[entity];
	}
	inline size_t size() const { return storage.size(); }

	template<typename CompType>
	class iterator {
	public:
		typedef iterator self_type;
		typedef Entity value_type;
		typedef Entity& reference;
		typedef Entity* pointer;
		typedef std::forward_iterator_tag iterator_category;
		iterator(Entity entity_, ComponentStorage<CompType, direct_indexing>& compStore) 
			: entity{ entity_ }, compStore{ compStore }, end{ (Entity)compStore.size() } {}
		self_type operator++(int dummy) {
			assert(entity < end);
			do {
				++entity;
			} while (entity < end && !compStore.containsVec[entity]);
			 //skip non valid entries
			assert(entity <= end);
			return *this;
		}
		self_type operator++() {
			self_type me = *this;
			operator++(0);
			return me;
		}
		reference operator*() {
			assert(entity < end&& compStore.containsVec[entity]);
			return entity;
		}
		pointer operator->() {
			assert(entity < end);
			return &entity;
		}
		bool operator==(self_type const& rhs) {
			return entity == rhs.entity;
		}
		bool operator!=(self_type const& rhs) {
			return entity != rhs.entity;
		}
		Entity data() {
			assert(entity < end);
			return compStore.storage[entity];
		}
	private:
		Entity entity; 
		ComponentStorage<CompType, direct_indexing>& compStore;
		const Entity end;
	};
	inline iterator<CompType> begin() {
		Entity entity = 0;
		while (entity < storage.size() && !contains(entity)) ++entity;
		return iterator<CompType>(entity, *this);
	}
	inline iterator<CompType> end() { return iterator<CompType>(storage.size(), *this); }
private:
	storage_t storage;
	std::vector<bool> containsVec;
};

static const int PAGE_BITS{ 10 };
static const int PAGE_SIZE{ 1 << PAGE_BITS };
static const int OFFSET_MASK{ ~(-1 << PAGE_BITS) };

static inline int page(Entity entity) {
	return entity >> PAGE_BITS;
}

static inline int offset(Entity entity) {
	return entity & OFFSET_MASK;
}

template<typename CompType>
class ComponentStorage<CompType, sparse_indexing> {

	struct Page {
		CompType data[PAGE_SIZE];
		int usedCount{ 0 };
	};
public:
	using Component = CompType;
	using storage_t = std::vector<CompType>;

	~ComponentStorage() {
		for (auto& page : pages) {
			if (page != nullptr)
				delete page;
		}
	}

	inline size_t memoryConsumtion() {
		return pages.size() * sizeof(Page*) + usedPages * PAGE_SIZE * sizeof(CompType);
	}

	inline size_t capacity() {
		return usedPages * PAGE_SIZE;
	}

	inline void updateMaxEntNum(size_t newEntNum) {
		if (newEntNum > containsVec.size())
			containsVec.resize(newEntNum, false);
		if (page(newEntNum-1)+1 > pages.size())
			pages.resize(page(newEntNum-1)+1, nullptr);
	}

	inline void insert(Entity entity, CompType const& comp) {
		if (pages[page(entity)] == nullptr)
			pages[page(entity)] = new Page;

		containsVec[entity] = true;

		pages[page(entity)]->data[offset(entity)] = comp;
		pages[page(entity)]->usedCount += 1;
	}

	inline void remove(Entity entity) {
		if (contains(entity)) {
			containsVec[entity] = false;

			pages[page(entity)]->usedCount -= 1;
			if (pages[page(entity)]->usedCount == 0) {
				delete pages[page(entity)];
				pages[page(entity)] = nullptr;
			}
				
		}
	}
	inline bool contains(Entity entity) const {
		return containsVec[entity];
	}
	inline CompType& get(Entity entity) {
		if (!contains(entity)) throw new std::exception();
		return pages[page(entity)]->data[offset(entity)];
	}
	inline CompType& operator[](Entity entity) {
		return get(entity);
	}
	inline size_t size() const {
		size_t size = 0;
		for (const auto& page : pages) {
			if (page != nullptr)
				size += page->usedCount;
		}
		return size;
	}

	template<typename CompType>
	class iterator {
	public:
		typedef iterator self_type;
		typedef Entity value_type;
		typedef Entity& reference;
		typedef Entity* pointer;
		typedef std::forward_iterator_tag iterator_category;
		iterator(Entity entity_, ComponentStorage<CompType, sparse_indexing>& compStore) 
			: entity{ entity_ }, compStore{ compStore }, end{ (Entity)compStore.containsVec.size() } {}
		self_type operator++(int dummy) {
			++entity;
			while (entity < end) {
				if (compStore.pages[page(entity)] == nullptr) // skip empty pages
				{
					entity = (page(entity) + 1) << PAGE_BITS;
				}
				else if (!compStore.contains(entity))
				{
					++entity;
				}
				else {
					break;
				}
			}
			if (entity > end)
				entity = end;
			return *this;
		}
		self_type operator++() {
			self_type me = *this;
			operator++(0);
			return me;
		}
		reference operator*() {
			return entity;
		}
		pointer operator->() {
			return &entity;
		}
		bool operator==(self_type const& rhs) {
			return entity == rhs.entity;
		}
		bool operator!=(self_type const& rhs) {
			return entity != rhs.entity;
		}
		Entity data() {
			return compStore.pages[page(entity)]->data[offset(entity)];
		}
	private:
		Entity entity;
		ComponentStorage<CompType, sparse_indexing>& compStore;
		const Entity end;
	};
	inline iterator<CompType> begin() {
		Entity entity = 0;
		while (entity < containsVec.size()) {
			if (pages[page(entity)] == nullptr) // skip empty pages
			{
				entity = (page(entity) + 1) << PAGE_BITS;
			}
			else if (!contains(entity))
			{
				++entity;
			}
			else {
				break;
			}
		}
		if (entity > containsVec.size())
			entity = containsVec.size();
		return iterator<CompType>(entity, *this);
	}
	inline iterator<CompType> end() { return iterator<CompType>(containsVec.size(), *this); }
private:
	size_t usedPages{ 0 };
	std::vector<Page*> pages;
	std::vector<bool> containsVec;
};


template<typename CompType>
class ComponentStorage<CompType, sparse_set> {
public:

	inline size_t memoryConsumtion() {
		return sparseTable.capacity() * sizeof(Entity) + denseTable.capacity() * sizeof(Entity); + storage.capacity() * sizeof(CompType);
	}

	inline void updateMaxEntNum(size_t newEntNum) {
		if (sparseTable.size() < newEntNum) {
			sparseTable.resize(newEntNum, 0xFFFFFFFF);
		}
	}

	inline void insert(Entity entity, CompType const& comp) {
		assert(!contains(entity));
		assert(entity < sparseTable.size());
		
		denseTable.push_back(entity);
		storage.push_back(comp);
		sparseTable.at(entity) = denseTable.size() - 1;

		assert(storage.size() == denseTable.size());
	}

	inline void remove(Entity entity) {
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

	inline bool contains(Entity entity) const {
		return sparseTable[entity] != 0xFFFFFFFF;
	}

	inline CompType& get(Entity entity) {
		return storage[sparseTable[entity]];
	}
	inline CompType& operator[](Entity entity) {
		return get(entity);
	}
	inline size_t size() const {
		return denseTable.size();
	}
	template<typename CompType>
	class iterator {
	public:
		typedef iterator self_type;
		typedef Entity value_type;
		typedef Entity& reference;
		typedef Entity* pointer;
		typedef std::forward_iterator_tag iterator_category;
		iterator(Entity denseTableIndex, ComponentStorage<CompType, sparse_set>& compStore) 
			: denseTableIndex{ denseTableIndex }, compStore{ compStore }, end{ (Entity)compStore.denseTable.size() } {}
		inline self_type operator++(int dummy) {
			assert(denseTableIndex < end);
			++denseTableIndex;
			return *this;
		}
		inline self_type operator++() {
			self_type me = *this;
			operator++(0);
			return me;
		}
		inline reference operator*() {
			return compStore.denseTable[denseTableIndex];
		}
		inline pointer operator->() {
			return &compStore.denseTable[denseTableIndex];
		}
		inline bool operator==(self_type const& rhs) {
			return denseTableIndex == denseTableIndex;
		}
		inline bool operator!=(self_type const& rhs) {
			return denseTableIndex != rhs.denseTableIndex;
		}
		inline CompType data() {
			return compStore.storage[denseTableIndex];
		}
	private:
		Entity denseTableIndex; 
		ComponentStorage<CompType, sparse_set>& compStore;
		const Entity end;
	};
	inline iterator<CompType> begin() { return iterator<CompType>(0, *this); }
	inline iterator<CompType> end()   { return iterator<CompType>(denseTable.size(), *this); }
private:
	std::vector<Entity> sparseTable;
	std::vector<Entity> denseTable;
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

	inline void insert(Entity entity, CompType const& comp) {
		// find next bugger entity index
		Entity nextBiggerEntityIndex = entity + 1;
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

	inline void remove(Entity entity) {
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

	inline bool contains(Entity entity) const {
#ifdef DEBUG_COMPONENT_STORAGE
		return indexTable.at(entity) != 0xFFFFFFFF;
#else
		return indexTable[entity] != 0xFFFFFFFF;
#endif
	}

	inline CompType& get(Entity entity) {
		assert(contains(entity));
		return storage[indexTable[entity]];
	}
	inline CompType& operator[](Entity entity) {
		return get(entity);
	}
	inline size_t size() const {
		return storage.size();
	}
	template<typename CompType>
	class iterator {
	public:
		typedef iterator self_type;
		typedef Entity value_type;
		typedef Entity& reference;
		typedef Entity* pointer;
		typedef std::forward_iterator_tag iterator_category;
		iterator(Entity entity, ComponentStorage<CompType, sorted_lookup_table>& compStore) 
			: entity{ entity }, compStore{ compStore }, end{ (Entity)compStore.indexTable.size() } {}
		inline self_type operator++(int dummy) {
			assert(entity < end);
			++entity;
			while (entity < end && !compStore.contains(entity)) ++entity;
			return *this;
		}
		inline self_type operator++() {
			self_type me = *this;
			operator++(0);
			return me;
		}
		inline reference operator*() {
			return entity;
		}
		inline pointer operator->() {
			return &entity;
		}
		inline bool operator==(self_type const& rhs) {
			return entity == rhs.entity;
		}
		inline bool operator!=(self_type const& rhs) {
			return entity != rhs.entity;
		}
		inline CompType data() {
			return compStore.storage[compStore.indexTable[entity]];
		}
	private:
		Entity entity;
		ComponentStorage<CompType, sorted_lookup_table>& compStore;
		const Entity end;
	};
	inline iterator<CompType> begin() {
		Entity ent = 0;
		while (ent < indexTable.size() && !contains(ent)) ++ent;
		return iterator<CompType>(ent, *this); 
	}
	inline iterator<CompType> end() { return iterator<CompType>(indexTable.size(), *this); }
private:
	std::vector<Entity> indexTable;
	storage_t storage;
};


template< size_t I, typename T, typename Tuple_t>
constexpr size_t index_in_storagetuple_fn() {
	static_assert(I < std::tuple_size<Tuple_t>::value, "the given component type is unknown");

	typedef typename std::tuple_element<I, Tuple_t>::type el;
	if constexpr (std::is_same<ComponentStorage<T, direct_indexing>, el>::value
		|| std::is_same<ComponentStorage<T, sparse_indexing>, el>::value
		|| std::is_same<ComponentStorage<T, sorted_lookup_table>, el>::value
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