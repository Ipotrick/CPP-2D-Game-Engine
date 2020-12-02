#pragma once
#include <utility>
#include <functional>	// std::invoke
#include <cassert>
#include <iterator>
#include <variant>
#include <tuple>
#include <vector>

#include "EntityTypes.hpp"

#ifdef _DEBUG
#define DEBUG_COMPONENT_STORAGE
#endif

#define DEBUG_COMPONENT_STORAGE

#ifdef DEBUG_COMPONENT_STORAGE
#define csat(key) at(key)
#define compStoreAssert(x) if (!(x)) throw new std::exception()
#else
#define csat(key) operator[](key)
#define compStoreAssert(x)
#endif

struct CompData { };

template<typename CompType>
class ComponentStorageBase {
public:
	// meta:
	void updateMaxEntNum(size_t newEntNum) { static_assert(false); };
	size_t memoryConsumtion() { static_assert(false); };
	size_t size() const { static_assert(false); };

	// access:
	void insert(EntityHandleIndex entity, CompType const& comp) { static_assert(false); };
	void remove(EntityHandleIndex entity) { static_assert(false); };
	bool contains(EntityHandleIndex entity) const { static_assert(false); };
	CompType& get(EntityHandleIndex entity) { static_assert(false); };
	const CompType& get(EntityHandleIndex entity) const { static_assert(false); };
};

/*----------------------------------------------------------------------------------*/
/*---------------------------------Direct-Indexing----------------------------------*/
/*----------------------------------------------------------------------------------*/

template<typename CompType>
class ComponentStorageDirectIndexing : public ComponentStorageBase<CompType> {
public:
	// meta:
	void updateMaxEntNum(size_t newEntNum)
	{
		if (containsVec.size() < newEntNum) {
			containsVec.resize(newEntNum, false);
		}
	}
	size_t memoryConsumtion() {
		return storage.capacity() * sizeof(CompType);
	}
	size_t size() const { return storage.size(); }

	// access:
	void insert(EntityHandleIndex entity, CompType const& comp)
	{
		compStoreAssert(!contains(entity));
		if (entity >= containsVec.size()) containsVec.resize(entity + 1, false);
		containsVec[entity] = true;
		if (entity < storage.size()) {
			storage[entity] = comp;
		}
		else if (entity == storage.size()) {
			storage.push_back(comp);
		}
		else if (entity > storage.size()) {
			storage.resize(entity + 1, CompType());
			storage[entity] = comp;
		}
	}
	void remove(EntityHandleIndex entity)
	{
		compStoreAssert(contains(entity));
		containsVec[entity] = false;
	}
	bool contains(EntityHandleIndex entity) const
	{
		compStoreAssert(entity < containsVec.size());
		return containsVec[entity];
	}
	CompType& get(EntityHandleIndex entity)
	{
		return storage.csat(entity);
	}
	const CompType& get(EntityHandleIndex entity) const
	{
		return storage.csat(entity);
	}
	template<typename CompType>
	class iterator {
	public:
		typedef iterator self_type;
		typedef EntityHandleIndex value_type;
		typedef EntityHandleIndex& reference;
		typedef EntityHandleIndex* pointer;
		typedef std::forward_iterator_tag iterator_category;
		iterator(EntityHandleIndex entity_, ComponentStorageDirectIndexing<CompType>& compStore)
			: entity{ entity_ }, compStore{ compStore }, end{ (EntityHandleIndex)compStore.size() } {}
		self_type operator++()
		{
			compStoreAssert(entity < end);
			do {
				++entity;
			} while (entity < end && !compStore.containsVec[entity]);
			//skip non valid entries
			return *this;
		}
		self_type operator++(int dummy)
		{
			self_type me = *this;
			operator++(0);
			return me;
		}
		reference operator*()
		{
			compStoreAssert(entity < end&& compStore.containsVec[entity]);
			return entity;
		}
		pointer operator->()
		{
			compStoreAssert(entity < end);
			return &entity;
		}
		bool operator==(self_type const& rhs)
		{
			return entity == rhs.entity;
		}
		bool operator!=(self_type const& rhs)
		{
			return entity != rhs.entity;
		}
		CompType& data()
		{
			compStoreAssert(entity < end);
			return compStore.storage[entity];
		}
	private:
		EntityHandleIndex entity;
		ComponentStorageDirectIndexing<CompType>& compStore;
		const EntityHandleIndex end;
	};
	iterator<CompType> begin()
	{
		EntityHandleIndex entity = 0;
		while (entity < storage.size() && !contains(entity)) ++entity;
		return iterator<CompType>(entity, *this);
	}
	iterator<CompType> end() { return iterator<CompType>(storage.size(), *this); }
private:
	std::vector<CompType> storage;
	std::vector<bool> containsVec;
};

/*----------------------------------------------------------------------------------*/
/*---------------------------------Paged-Indexing-----------------------------------*/
/*----------------------------------------------------------------------------------*/

template<typename CompType>
class ComponentStoragePagedIndexing : public ComponentStorageBase<CompType> {
public:
	ComponentStoragePagedIndexing() = default;
	ComponentStoragePagedIndexing(ComponentStoragePagedIndexing<CompType> const& rhs)
	{
		operator=(rhs);
	}

	// meta:
	void updateMaxEntNum(size_t newEntNum)
	{
		if (newEntNum > containsVec.size()) {
			containsVec.resize(newEntNum, false);
		}
		if (page(newEntNum - 1) + 1 > pages.size()) {
			pages.resize(page(newEntNum - 1) + 1);
		}
	}
	size_t memoryConsumtion()
	{
		//return pages.size() * sizeof(Page*) + usedPages * PAGE_SIZE * sizeof(CompType);
		return containsVec.size();
	}
	size_t size() const 
	{
		return m_size;
	};
	void operator=(const ComponentStoragePagedIndexing<CompType>& rhs)
	{
		this->containsVec = rhs.containsVec;

		this->pages.resize(rhs.pages.size());
		for (int i = 0; i < this->pages.size(); i++) {
			if (rhs.pages[i]) {
				this->pages[i] = std::make_unique<Page>(*rhs.pages[i]);
			}
			else if (this->pages[i]) {
				this->pages[i].reset();
			}
		}
	}

	// access:
	void insert(EntityHandleIndex entity, CompType const& comp)
	{
		compStoreAssert(!contains(entity));
		updateMaxEntNum(entity + 1);


		if (!pages[page(entity)]) {
			pages[page(entity)] = std::make_unique<Page>();
		}

		containsVec[entity] = true;

		pages[page(entity)]->data[offset(entity)] = comp;
		pages[page(entity)]->usedCount += 1;
		++m_size;
	}
	void remove(EntityHandleIndex entity)
	{
		compStoreAssert(contains(entity));
		containsVec[entity] = false;

		pages[page(entity)]->usedCount -= 1;
		if constexpr (DELETE_EMPTY_PAGES) {
			if (pages[page(entity)]->usedCount == 0) {
				pages[page(entity)].reset();
			}
		}
		--m_size;
	}
	bool contains(EntityHandleIndex entity) const
	{
		//compStoreAssert(entity < containsVec.size());
		return entity < containsVec.size() && containsVec[entity];
	}
	CompType& get(EntityHandleIndex entity)
	{
		return pages.csat(page(entity))->data.csat(offset(entity));
	}
	const CompType& get(EntityHandleIndex entity) const
	{
		return pages.csat(page(entity))->data.csat(offset(entity));
	}
	template<typename CompType>
	class iterator {
	public:
		typedef iterator self_type;
		typedef EntityHandleIndex value_type;
		typedef EntityHandleIndex& reference;
		typedef EntityHandleIndex* pointer;
		typedef std::forward_iterator_tag iterator_category;
		iterator(EntityHandleIndex entity_, ComponentStoragePagedIndexing<CompType>& compStore)
			: entity{ entity_ }, compStore{ compStore }, end{ (EntityHandleIndex)compStore.containsVec.size() } {}
		self_type operator++()
		{
			++entity;
			while (entity < end) {
				if (compStore.pages[page(entity)] == nullptr) // skip empty pages
				{
					entity = (page(entity) + 1) << PAGE_BITS;
				}
				else if (!compStore.contains(entity)) {
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
		self_type operator++(int dummy)
		{
			self_type me = *this;
			operator++(0);
			return me;
		}
		reference operator*()
		{
			return entity;
		}
		pointer operator->()
		{
			return &entity;
		}
		bool operator==(self_type const& rhs)
		{
			return entity == rhs.entity;
		}
		bool operator!=(self_type const& rhs)
		{
			return entity != rhs.entity;
		}
		CompType& data()
		{
			return compStore.pages[page(entity)]->data[offset(entity)];
		}
	private:
		EntityHandleIndex entity;
		ComponentStoragePagedIndexing<CompType>& compStore;
		const EntityHandleIndex end;
	};
	iterator<CompType> begin()
	{
		EntityHandleIndex entity = 0;
		while (entity < containsVec.size()) {
			if (!pages[page(entity)]) // skip empty pages
			{
				entity = (page(entity) + 1) << PAGE_BITS;
			}
			else if (!contains(entity)) {
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
	iterator<CompType> end() { return iterator<CompType>(containsVec.size(), *this); }

private:
	static const int PAGE_BITS{ 7 };
	static const int PAGE_SIZE{ 1 << PAGE_BITS };
	static const int OFFSET_MASK{ ~(-1 << PAGE_BITS) };

	static int page(EntityHandleIndex entity)
	{
		return entity >> PAGE_BITS;
	}
	static int offset(EntityHandleIndex entity)
	{
		return entity & OFFSET_MASK;
	}
	static const bool DELETE_EMPTY_PAGES{ true };
	struct Page {
		size_t usedCount{ 0 };
		std::array<CompType, PAGE_SIZE> data;
	};

	size_t usedPages{ 0 };
	size_t m_size{ 0 };
	std::vector<std::unique_ptr<Page>> pages;
	std::vector<bool> containsVec;
};

/*----------------------------------------------------------------------------------*/
/*------------------------------------Paged-Set-------------------------------------*/
/*----------------------------------------------------------------------------------*/

template<typename CompType>
class ComponentStoragePagedSet : public ComponentStorageBase<CompType> {
public:
	ComponentStoragePagedSet() = default;
	ComponentStoragePagedSet(ComponentStoragePagedSet<CompType> const& rhs)
	{
		operator=(rhs);
	}
	// meta:
	void updateMaxEntNum(EntityHandleIndex newEntNum)
	{
		if (size_t(page(newEntNum - 1) + 1) > pages.size()) {
			pages.resize(size_t(page(newEntNum - 1) + 1));
		}
	}
	size_t memoryConsumtion()
	{
		size_t s = denseTable.capacity() * sizeof(EntityHandleIndex); +storage.capacity() * sizeof(CompType) + pages.size() * sizeof(Page*);
		for (auto& page : pages) {
			if (page != nullptr) {
				s += sizeof(Page);
			}
		}
		return s;
	}
	size_t size() const
	{
		return denseTable.size();
	}
	void operator=(const ComponentStoragePagedSet<CompType>& rhs)
	{
		this->denseTable = rhs.denseTable;
		this->storage = rhs.storage;

		this->pages.resize(rhs.pages.size());
		for (int i = 0; i < this->pages.size(); i++) {
			if (rhs.pages[i]) {
				this->pages[i] = std::make_unique<Page>(*rhs.pages[i]);
			}
			else if (this->pages[i]) {
				this->pages[i].reset();
			}
		}
	}

	// access:
	void insert(EntityHandleIndex entity, CompType const& comp)
	{
		compStoreAssert(!contains(entity));
		updateMaxEntNum(entity + 1);
		denseTable.push_back(entity);
		storage.push_back(comp);

		if (!pages[page(entity)]) {
			pages[page(entity)] = std::make_unique<Page>();
		}
		sparseTable(entity) = (uint32_t)denseTable.size() - 1;
		pages[page(entity)]->usedCount++;
	}
	void remove(EntityHandleIndex entity)
	{
		compStoreAssert(contains(entity));
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
		compStoreAssert(pages[page(entity)]->usedCount >= 0);
		if constexpr (DELETE_EMPTY_PAGES) {
			if (pages[page(entity)]->usedCount == 0) {
				pages[page(entity)].reset();
			}
		}
	}
	bool contains(EntityHandleIndex entity) const
	{
		return page(entity) < pages.size() && pages[page(entity)] != nullptr && sparseTable(entity) != 0xFFFFFFFF;
	}
	CompType& get(EntityHandleIndex entity)
	{
		return storage.csat(pages.at(page(entity))->data.csat(offset(entity)));
	}
	const CompType& get(EntityHandleIndex entity) const
	{
		return storage.csat(pages.at(page(entity))->data.csat(offset(entity)));
	}

	template<typename CompType>
	class iterator {
	public:
		typedef iterator self_type;
		typedef EntityHandleIndex value_type;
		typedef EntityHandleIndex& reference;
		typedef EntityHandleIndex* pointer;
		typedef std::forward_iterator_tag iterator_category;
		iterator(EntityHandleIndex denseTableIndex, ComponentStoragePagedSet<CompType>& compStore)
			: denseTableIndex{ denseTableIndex }, compStore{ compStore } {}
		self_type operator++()
		{
			compStoreAssert(denseTableIndex < compStore.denseTable.size());
			++denseTableIndex;
			return *this;
		}
		self_type operator++(int dummy)
		{
			self_type me = *this;
			operator++(0);
			return me;
		}
		reference operator*()
		{
			return compStore.denseTable[denseTableIndex];
		}
		pointer operator->()
		{
			return &compStore.denseTable[denseTableIndex];
		}
		bool operator==(self_type const& rhs)
		{
			return denseTableIndex == denseTableIndex;
		}
		bool operator!=(self_type const& rhs)
		{
			return denseTableIndex != rhs.denseTableIndex;
		}
		CompType& data()
		{
			return compStore.storage[denseTableIndex];
		}
	private:
		EntityHandleIndex denseTableIndex;
		ComponentStoragePagedSet<CompType>& compStore;
	};
	iterator<CompType> begin() { return iterator<CompType>(0, *this); }
	iterator<CompType> end() { return iterator<CompType>(denseTable.size(), *this); }
private:
	static const int PAGE_BITS{ 7 };
	static const int PAGE_SIZE{ 1 << PAGE_BITS };
	static const int OFFSET_MASK{ ~(-1 << PAGE_BITS) };

	static int page(EntityHandleIndex entity)
	{
		return entity >> PAGE_BITS;
	}
	static int offset(EntityHandleIndex entity)
	{
		return entity & OFFSET_MASK;
	}
	static const bool DELETE_EMPTY_PAGES{ true };
	struct Page {
		Page()
		{
			for (auto& el : data)
				el = 0xFFFFFFFF;
		}
		size_t usedCount{ 0 };
		std::array<uint32_t, PAGE_SIZE> data;
	};

	uint32_t& sparseTable(EntityHandleIndex ent)
	{
		return pages.csat(page(ent))->data.csat(offset(ent));
	}
	const uint32_t& sparseTable(EntityHandleIndex ent) const
	{
		return pages.csat(page(ent))->data.csat(offset(ent));
	}
	std::vector<std::unique_ptr<Page>> pages;
	std::vector<EntityHandleIndex> denseTable;
	std::vector<CompType> storage;
};

/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/