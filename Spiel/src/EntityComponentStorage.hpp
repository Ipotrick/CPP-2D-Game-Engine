#pragma once
#include <utility>
#include <functional>	// std::invoke
#include <cassert>
#include <iterator>
#include <variant>
#include <tuple>
#include <vector>

#include "EntityTypes.hpp"
#define DEBUG_COMPONENT_STORAGE

struct CompData {
};

using storage_t = int;

constexpr storage_t direct_indexing = 0;
constexpr storage_t paged_indexing = 1;
constexpr storage_t sparse_set = 2;
constexpr storage_t paged_set = 3;

template<typename CompType, storage_t storageType>
class ComponentStorage {
public:
	constexpr storage_t storageType() const { return storageType; }
	using Component = CompType;
	inline void updateMaxEntNum(size_t newEntNum);
	inline size_t memoryConsumtion();
	inline void insert(EntityHandleIndex entity, CompType const& comp);
	inline void remove(EntityHandleIndex entity);
	inline bool contains(EntityHandleIndex entity) const;
	inline CompType& get(EntityHandleIndex entity);
	inline const CompType& get(EntityHandleIndex entity) const;
	inline CompType& operator[](EntityHandleIndex ent);
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
	};
	inline iterator begin();
	inline iterator end();
	inline std::vector<std::pair<EntityHandleIndex, CompType>> dump() const;
};

template<typename CompType>
class ComponentStorage<CompType, direct_indexing>{
public:
	using Component = CompType;
	constexpr storage_t storageType() const { return direct_indexing; }

	inline size_t memoryConsumtion() {
		return storage.capacity() * sizeof(CompType);
	}
	inline void updateMaxEntNum(size_t newEntNum) {
		if (containsVec.size() < newEntNum) {
			containsVec.resize(newEntNum, false);
		}
	}
	inline void insert(EntityHandleIndex entity, CompType const& comp) {
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
	inline void remove(EntityHandleIndex entity) {
		if (contains(entity)) {
			containsVec[entity] = false;
		}
	}
	inline bool contains(EntityHandleIndex entity) const {
		return containsVec[entity];
	}
	inline CompType& get(EntityHandleIndex entity) {
#ifdef DEBUG_COMPONENT_STORAGE
		assert(contains(entity));
		return storage.at(entity);
#else
		return storage[entity];
#endif
	}
	inline const CompType& get(EntityHandleIndex entity) const
	{
#ifdef DEBUG_COMPONENT_STORAGE
		assert(contains(entity));
		return storage.at(entity);
#else
		return storage[entity];
#endif
	}
	inline CompType& operator[](EntityHandleIndex entity) {
		assert(contains(entity));
		return get(entity);
	}
	inline size_t size() const { return storage.size(); }

	template<typename CompType>
	class iterator {
	public:
		typedef iterator self_type;
		typedef EntityHandleIndex value_type;
		typedef EntityHandleIndex& reference;
		typedef EntityHandleIndex* pointer;
		typedef std::forward_iterator_tag iterator_category;
		iterator(EntityHandleIndex entity_, ComponentStorage<CompType, direct_indexing>& compStore) 
			: entity{ entity_ }, compStore{ compStore }, end{ (EntityHandleIndex)compStore.size() } {}
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
		CompType& data() {
			assert(entity < end);
			return compStore.storage[entity];
		}
	private:
		EntityHandleIndex entity; 
		ComponentStorage<CompType, direct_indexing>& compStore;
		const EntityHandleIndex end;
	};
	inline iterator<CompType> begin() {
		EntityHandleIndex entity = 0;
		while (entity < storage.size() && !contains(entity)) ++entity;
		return iterator<CompType>(entity, *this);
	}
	inline iterator<CompType> end() { return iterator<CompType>(storage.size(), *this); }
	inline std::vector<std::pair<EntityHandleIndex, CompType>> dump() const
	{
		std::vector<std::pair<EntityHandleIndex, CompType>> res;
		for (EntityHandleIndex ent = 0; ent < containsVec.size(); ent++) {
			if (contains(ent)) {
				res.emplace_back(ent, get(ent));
			}
		}
		return res;
	}
private:
	std::vector<CompType> storage;
	std::vector<bool> containsVec;
};

template<typename CompType>
class ComponentStorage<CompType, paged_indexing> {
	static const int PAGE_BITS{ 7 };
	static const int PAGE_SIZE{ 1 << PAGE_BITS };
	static const int OFFSET_MASK{ ~(-1 << PAGE_BITS) };

	static inline int page(EntityHandleIndex entity)
	{
		return entity >> PAGE_BITS;
	}
	static inline int offset(EntityHandleIndex entity)
	{
		return entity & OFFSET_MASK;
	}
	static const bool DELETE_EMPTY_PAGES{ true };
	class Page {
	public:
		std::array<CompType, PAGE_SIZE> data;
		int usedCount{ 0 };
	};
public:
	using Component = CompType;

	~ComponentStorage() {
		for (auto& page : pages) {
			if (page != nullptr) {
				delete page;
			}
		}
	}
	constexpr storage_t storageType() const { return paged_indexing; }

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
	inline void insert(EntityHandleIndex entity, CompType const& comp) {
		if (pages[page(entity)] == nullptr) {
			pages[page(entity)] = new Page;
		}

		containsVec[entity] = true;

		pages[page(entity)]->data[offset(entity)] = comp;
		pages[page(entity)]->usedCount += 1;
		++m_size;
	}
	inline void remove(EntityHandleIndex entity) {
		if (contains(entity)) {
			containsVec[entity] = false;

			pages[page(entity)]->usedCount -= 1;
			if constexpr (DELETE_EMPTY_PAGES) {
				if (pages[page(entity)]->usedCount == 0) {
					delete pages[page(entity)];
					pages[page(entity)] = nullptr;
				}
			}
			--m_size;
		}
	}
	inline bool contains(EntityHandleIndex entity) const {
		return containsVec[entity];
	}
	inline CompType& get(EntityHandleIndex entity) {
#ifdef DEBUG_COMPONENT_STORAGE
		return pages.at(page(entity))->data.at(offset(entity));
#else
		return pages[page(entity)]->data[offset(entity)];
#endif
	}
	inline const CompType& get(EntityHandleIndex entity) const
	{
#ifdef DEBUG_COMPONENT_STORAGE
		return pages.at(page(entity))->data.at(offset(entity));
#else
		return pages[page(entity)]->data[offset(entity)];
#endif
	}
	inline CompType& operator[](EntityHandleIndex entity) {
		return get(entity);
	}
	inline size_t size() const {
		return m_size;
	}

	template<typename CompType>
	class iterator {
	public:
		typedef iterator self_type;
		typedef EntityHandleIndex value_type;
		typedef EntityHandleIndex& reference;
		typedef EntityHandleIndex* pointer;
		typedef std::forward_iterator_tag iterator_category;
		iterator(EntityHandleIndex entity_, ComponentStorage<CompType, paged_indexing>& compStore) 
			: entity{ entity_ }, compStore{ compStore }, end{ (EntityHandleIndex)compStore.containsVec.size() } {}
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
		CompType& data() {
			return compStore.pages[page(entity)]->data[offset(entity)];
		}
	private:
		EntityHandleIndex entity;
		ComponentStorage<CompType, paged_indexing>& compStore;
		const EntityHandleIndex end;
	};
	inline iterator<CompType> begin() {
		EntityHandleIndex entity = 0;
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
	inline std::vector<std::pair<EntityHandleIndex, CompType>> dump() const
	{
		std::vector<std::pair<EntityHandleIndex, CompType>> res;
		for (EntityHandleIndex ent = 0; ent < containsVec.size(); ent++) {
			if (contains(ent)) {
				res.emplace_back( ent, get(ent) );
			}
		}
		return res;
	}
private:
	size_t usedPages{ 0 };
	size_t m_size{ 0 };
	std::vector<Page*> pages;
	std::vector<bool> containsVec;
};

template<typename CompType>
class ComponentStorage<CompType, sparse_set> {
public:
	using Component = CompType;
	constexpr storage_t storageType() const { return sparse_set; }
	inline size_t memoryConsumtion()
	{
		return sparseTable.capacity() * sizeof(EntityHandleIndex) + denseTable.capacity() * sizeof(EntityHandleIndex); +storage.capacity() * sizeof(CompType);
	}
	inline void updateMaxEntNum(size_t newEntNum)
	{
		if (sparseTable.size() < newEntNum) {
			sparseTable.resize(newEntNum, 0xFFFFFFFF);
		}
	}
	inline void insert(EntityHandleIndex entity, CompType const& comp)
	{
		assert(!contains(entity));
		assert(entity < sparseTable.size());

		denseTable.push_back(entity);
		storage.push_back(comp);
		sparseTable.at(entity) = denseTable.size() - 1;

		assert(storage.size() == denseTable.size());
	}
	inline void remove(EntityHandleIndex entity)
	{
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
	inline bool contains(EntityHandleIndex entity) const
	{
		return sparseTable[entity] != 0xFFFFFFFF;
	}
	inline CompType& get(EntityHandleIndex entity)
	{
#ifdef DEBUG_COMPONENT_STORAGE
		return storage.at(sparseTable.at(entity));
#else
		return storage[sparseTable[entity]];
#endif
	}
	inline const CompType& get(EntityHandleIndex entity) const
	{
#ifdef DEBUG_COMPONENT_STORAGE
		return storage.at(sparseTable.at(entity));
#else
		return storage[sparseTable[entity]];
#endif
	}
	inline CompType& operator[](EntityHandleIndex entity)
	{
		return get(entity);
	}
	inline size_t size() const
	{
		return denseTable.size();
	}
	template<typename CompType>
	class iterator {
	public:
		typedef iterator self_type;
		typedef EntityHandleIndex value_type;
		typedef EntityHandleIndex& reference;
		typedef EntityHandleIndex* pointer;
		typedef std::forward_iterator_tag iterator_category;
		iterator(EntityHandleIndex denseTableIndex, ComponentStorage<CompType, sparse_set>& compStore)
			: denseTableIndex{ denseTableIndex }, compStore{ compStore } {}
		inline self_type operator++(int dummy)
		{
			assert(denseTableIndex < compStore.denseTable.size());
			++denseTableIndex;
			return *this;
		}
		inline self_type operator++()
		{
			self_type me = *this;
			operator++(0);
			return me;
		}
		inline reference operator*()
		{
			return compStore.denseTable[denseTableIndex];
		}
		inline pointer operator->()
		{
			return &compStore.denseTable[denseTableIndex];
		}
		inline bool operator==(self_type const& rhs)
		{
			return denseTableIndex == denseTableIndex;
		}
		inline bool operator!=(self_type const& rhs)
		{
			return denseTableIndex != rhs.denseTableIndex;
		}
		inline CompType& data()
		{
			return compStore.storage[denseTableIndex];
		}
	private:
		EntityHandleIndex denseTableIndex;
		ComponentStorage<CompType, sparse_set>& compStore;
	};
	inline iterator<CompType> begin() { return iterator<CompType>(0, *this); }
	inline iterator<CompType> end() { return iterator<CompType>(denseTable.size(), *this); }
	inline std::vector<std::pair<EntityHandleIndex, CompType>> dump() const
	{
		std::vector<std::pair<EntityHandleIndex, CompType>> res;
		for (EntityHandleIndex ent : denseTable) {
			res.emplace_back(ent, get(ent));
		}
		return res;
	}
private:
	std::vector<EntityHandleIndex> sparseTable;
	std::vector<EntityHandleIndex> denseTable;
	std::vector<CompType> storage;
};

template<typename CompType>
class ComponentStorage<CompType, paged_set> {
	static const int PAGE_BITS{ 7 };
	static const int PAGE_SIZE{ 1 << PAGE_BITS };
	static const int OFFSET_MASK{ ~(-1 << PAGE_BITS) };

	static inline int page(EntityHandleIndex entity)
	{
		return entity >> PAGE_BITS;
	}
	static inline int offset(EntityHandleIndex entity)
	{
		return entity & OFFSET_MASK;
	}
	static const bool DELETE_EMPTY_PAGES{ false };
	struct Page {
		Page()
		{
			for (auto& el : data)
				el = 0xFFFFFFFF;
		}
		std::array<uint32_t, PAGE_SIZE> data;
		int usedCount{ 0 };
	};
public:
	using Component = CompType;
	constexpr storage_t storageType() const { return paged_set; }
	inline size_t memoryConsumtion() {
		size_t s = denseTable.capacity() * sizeof(EntityHandleIndex); +storage.capacity() * sizeof(CompType) + pages.size() * sizeof(Page*);
		for (auto& page : pages) {
			if (page != nullptr) {
				s += sizeof(Page);
			}
		}
		return s;
	}
	inline void updateMaxEntNum(size_t newEntNum) {
		if (page(newEntNum - 1) + 1 > pages.size()) {
			pages.resize(page(newEntNum - 1) + 1, nullptr);
		}
	}
	inline void insert(EntityHandleIndex entity, CompType const& comp) {
		assert(!contains(entity));
		
		denseTable.push_back(entity);
		storage.push_back(comp);

		if (pages[page(entity)] == nullptr) {
			pages[page(entity)] = new Page;
		}
		sparseTable(entity) = (uint32_t)denseTable.size() - 1;
		pages[page(entity)]->usedCount++;

		assert(contains(entity));
	}
	inline void remove(EntityHandleIndex entity) {
		if (contains(entity)) {
			if (entity == denseTable.back()) {
				sparseTable(entity) = 0xFFFFFFFF;
				pages[page(entity)]->usedCount--;
				denseTable.pop_back();
				storage.pop_back();
			}
			else {
				uint32_t slot = sparseTable(entity);
				sparseTable(entity) = 0xFFFFFFFF;
				pages[page(entity)]->usedCount--;
				EntityHandleIndex lastEnt = denseTable.back();
				denseTable.pop_back();
				sparseTable(lastEnt) = slot;
				denseTable.at(slot) = lastEnt;
				storage.at(slot) = storage.back();
				storage.pop_back();
			}
			assert(pages[page(entity)]->usedCount >= 0);
			if (DELETE_EMPTY_PAGES && pages[page(entity)]->usedCount == 0) {
				delete pages[page(entity)];
				pages[page(entity)] = nullptr;
			}
		}
	}
	inline bool contains(EntityHandleIndex entity) const {
		return pages.at(page(entity)) != nullptr && sparseTable(entity) != 0xFFFFFFFF;
	}
	inline CompType& get(EntityHandleIndex entity) {
#ifdef DEBUG_COMPONENT_STORAGE
		return storage.at(pages.at(page(entity))->data.at(offset(entity)));
#else
		return storage[pages[page(entity)]->data[offset(entity)]];
#endif
	}
	inline const CompType& get(EntityHandleIndex entity) const
	{
#ifdef DEBUG_COMPONENT_STORAGE
		return storage.at(pages.at(page(entity))->data.at(offset(entity)));
#else
		return storage[pages[page(entity)]->data[offset(entity)]];
#endif
	}
	inline CompType& operator[](EntityHandleIndex entity) {
		return get(entity);
	}
	inline size_t size() const {
		return denseTable.size();
	}
	template<typename CompType>
	class iterator {
	public:
		typedef iterator self_type;
		typedef EntityHandleIndex value_type;
		typedef EntityHandleIndex& reference;
		typedef EntityHandleIndex* pointer;
		typedef std::forward_iterator_tag iterator_category;
		iterator(EntityHandleIndex denseTableIndex, ComponentStorage<CompType, paged_set>& compStore) 
			: denseTableIndex{ denseTableIndex }, compStore{ compStore } {}
		inline self_type operator++(int dummy) {
			assert(denseTableIndex < compStore.denseTable.size());
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
		inline CompType& data() {
			return compStore.storage[denseTableIndex];
		}
	private:
		EntityHandleIndex denseTableIndex; 
		ComponentStorage<CompType, paged_set>& compStore;
	};
	inline iterator<CompType> begin() { return iterator<CompType>(0, *this); }
	inline iterator<CompType> end()   { return iterator<CompType>(denseTable.size(), *this); }
	inline std::vector<std::pair<EntityHandleIndex, CompType>> dump() const
	{
		std::vector<std::pair<EntityHandleIndex, CompType>> res;
		for (EntityHandleIndex ent : denseTable) {
			res.emplace_back( ent, get(ent) );
		}
		return res;
	}
private:
	inline uint32_t& sparseTable(EntityHandleIndex ent)
	{
#ifdef DEBUG_COMPONENT_STORAGE
		return pages.at(page(ent))->data.at(offset(ent));
#else
		return pages[page(ent)]->data[offset(ent)];
#endif
	}
	inline const uint32_t& sparseTable(EntityHandleIndex ent) const
	{
#ifdef DEBUG_COMPONENT_STORAGE
		return pages.at(page(ent))->data.at(offset(ent));
#else
		return pages[page(ent)]->data[offset(ent)];
#endif
	}
	std::vector<Page*> pages;
	std::vector<EntityHandleIndex> denseTable;
	std::vector<CompType> storage;
};

template< size_t I, typename T, typename Tuple_t>
constexpr size_t index_in_storagetuple_fn() {
	static_assert(I < std::tuple_size<Tuple_t>::value, "the given component type is unknown");

	typedef typename std::tuple_element<I, Tuple_t>::type el;
	if constexpr (std::is_same<ComponentStorage<T, direct_indexing>, el>::value
		|| std::is_same<ComponentStorage<T, paged_indexing>, el>::value
		|| std::is_same<ComponentStorage<T, sparse_set>, el>::value
		|| std::is_same<ComponentStorage<T, paged_set>, el>::value) {
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