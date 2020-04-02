#pragma once
#include <cassert>
#include <iterator>

#include <vector>

#include "robin_hood.h"

struct CompData {

};

using ent_id_t = uint32_t;

using storage_t = int;

constexpr storage_t storage_index_t = 0;
constexpr storage_t storage_hash_t = 1;

template<typename CompType, storage_t storageType>
class ComponentStorage {
public:
	inline void registrate(ent_id_t entity, CompType const& comp);
	inline void deregistrate(ent_id_t entity);
	inline bool isRegistrated(ent_id_t entity) const;
	inline CompType& getComponent(ent_id_t entity);
	inline CompType const& getComponent(ent_id_t entity) const;
	
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
class ComponentStorage<CompType, storage_hash_t> {
public:
	using storage_t = robin_hood::unordered_map<uint32_t, CompType>;

	inline void registrate(ent_id_t entity, CompType const& comp) {
		storage.insert({ entity, comp });
	}
	inline void deregistrate(ent_id_t entity) {
		auto res = storage.find(entity);
		if (res != storage.end()) {
			storage.erase(res);
		}
	}
	inline bool isRegistrated(ent_id_t entity) const {
		auto res = storage.find(entity);
		if (res != storage.end() && res->first == entity) {
			return true;
		}
		return false;
	}
	inline CompType& getComponent(ent_id_t entity) {
		assert(isRegistrated(entity));
		return storage[entity];
	}
	inline CompType const& getComponent(ent_id_t entity) const {
		assert(isRegistrated(entity));
		return storage[entity];
	}
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
class ComponentStorage<CompType, storage_index_t>{
public:
	using storage_t = std::vector<std::pair<bool, CompType>>;

	inline void registrate(ent_id_t entity, CompType const& comp) {
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
	inline void deregistrate(ent_id_t entity) {
		if (entity < storage.size()) {
			storage[entity].first = false;
		}
	}
	inline bool isRegistrated(ent_id_t entity) const {
		if (entity < storage.size()) {
			return storage[entity].first;
		}
		else {
			return false;
		}
	}
	inline CompType& getComponent(ent_id_t entity) {
		assert(isRegistrated(entity));
		return storage[entity].second;
	}
	inline CompType const& getComponent(ent_id_t entity) const {
		assert(isRegistrated(entity));
		return storage[entity].second;
	}
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
		bool operator==(const self_type& rhs) {
			return entity == entity;
		}
		bool operator!=(const self_type& rhs) {
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