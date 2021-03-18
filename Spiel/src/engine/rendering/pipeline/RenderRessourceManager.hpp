#pragma once

#include <vector>
#include <mutex>
#include <cassert>
#include <ranges>
#include <concepts>
#include <array>

#include <robin_hood.h>

#include "../../types/ShortNames.hpp"

#include "RessourceHandle.hpp"

// TODO make concepts for THandle, TDescriptor, TRessource

template<typename T>
concept CRessourceHandle = requires(T a, T b)
{
	{ a == b } -> std::same_as<bool>;
	a.index;
	a.version;
	a.managerId;
};

template<typename T>
concept CDescriptor = requires(T a, T b)
{
	{ a == b } -> std::same_as<bool>;
	{ T{ a } } -> std::same_as<T>;
	{ T{ std::move(a) } } -> std::same_as<T>;
};

template<typename T>
concept CRenderRessource = requires(T a)
{
	{ a.load({}) } -> std::same_as<void>;
	{ a.reload() } -> std::same_as<void>;
};

template<
	CRessourceHandle THandle,
	CDescriptor TDescriptor,
	typename TCreator,
	CRenderRessource TRessource
>
class RenderRessourceManager {
public:
	class Backend {
	public:
		// Frontend Interface:

		Backend(u16 managerId) : MANAGER_ID{ managerId } {}

		void queueLoad(std::pair<u32, TDescriptor>&& res)
		{
			std::unique_lock l(mut);
			if (std::ranges::find(loadingQueueFront, res) == std::end(loadingQueueFront)) {
				loadingQueueFront.push_back(std::move(res));
			}
		}

		void queueCreate(std::pair<u32, TCreator>&& res)
		{
			std::unique_lock l(mut);
			if (std::ranges::find(createQueueFront, res) == std::end(createQueueFront)) {
				createQueueFront.push_back(std::move(res));
			}
		}

		void queueUnload(u32 index)
		{
			std::unique_lock l(mut);
			if (std::ranges::find(unloadingQueueFront, index) == std::end(unloadingQueueFront)) {
				unloadingQueueFront.push_back(index);
			}
		}

		/**
		 * pushes data from frontend buffer to the backend.
		 * clears the frontend buffer afterwards.
		 */
		void flush()
		{
			std::unique_lock l(mut);
			loadingQueueBack.insert(std::end(loadingQueueBack), std::begin(loadingQueueFront), std::end(loadingQueueFront));
			loadingQueueFront.clear();
			for (auto& create : createQueueFront) {
				createQueueBack.push_back(std::move(create));
			}
			createQueueFront.clear();
			unloadingQueueBack.insert(std::end(unloadingQueueBack), std::begin(unloadingQueueFront), std::end(unloadingQueueFront));
			unloadingQueueFront.clear();
		}

		// Backend Interface:
		std::unique_lock<std::mutex> makeLock() const 
		{ 
			return std::unique_lock<std::mutex>(mut); 
		}

		/**
		 * call BEFORE the draw
		 */
		void createAndLoadQueueExecute(std::unique_lock<std::mutex>& lock)
		{
			assert(lock.mutex() == &mut && lock.owns_lock());
			for (auto& [index, descriptor] : loadingQueueBack) {
				std::cout << "loading texture in index: " << index << " descriptor: " << descriptor << std::endl;
				assert(index <= ressources.size());
				if (index == ressources.size()) {
					ressources.emplace_back(TRessource{ descriptor }, 0, true);
				}
				else {
					ressources[index].value.load(descriptor);
					ressources[index].version += 1;
					ressources[index].exists = true;
				}
			}
			loadingQueueBack.clear();
			for (auto&& [index, create] : createQueueBack) {
				std::cout << "create texture in index: " << index << " create: " << create << std::endl;
				assert(index <= ressources.size());
				if (index == ressources.size()) {
					ressources.emplace_back(TRessource{ create }, 0, true);
				}
				else {
					ressources[index].value.load(create);
					ressources[index].version += 1;
					ressources[index].exists = true;
				}
			}
			createQueueBack.clear();
		}

		/**
		 * call AFTER the draw
		 */
		void unloadQueue(std::unique_lock<std::mutex>& lock)
		{
			assert(lock.mutex() == &mut && lock.owns_lock());
			for (u32 index : unloadingQueueBack) {
				ressources[index].value.reset();
				ressources[index].exists = false;
			}
			unloadingQueueBack.clear();
		}

		bool isLoaded(std::unique_lock<std::mutex>& lock, s32 index) const
		{
			assert(lock.mutex() == &mut && lock.owns_lock());
			return index < ressources.size() && ressources[index].value.loaded();
		}

		void reload(std::unique_lock<std::mutex>& lock, s32 index) const
		{
			assert(lock.mutex() == &mut && lock.owns_lock());
			ressources[index].value.reload();
		}

		const TRessource& get(std::unique_lock<std::mutex>& lock, s32 index) const
		{
			assert(lock.mutex() == &mut && lock.owns_lock());
			return ressources[index].value;
		}

		TRessource& get(std::unique_lock<std::mutex>& lock, s32 index)
		{
			assert(lock.mutex() == &mut && lock.owns_lock());
			return ressources[index].value;
		}

		bool isHandleValid(std::unique_lock<std::mutex>& lock, const THandle& handle) const
		{
			return
				handle.managerId == MANAGER_ID &&
				handle.index < ressources.size() &&
				handle.version == ressources[handle.index].version &&
				ressources[handle.index].exists;
		}
	private:
		const u16 MANAGER_ID;
		mutable std::mutex mut; 
		std::vector<std::pair<u32, TDescriptor>> loadingQueueFront;
		std::vector<std::pair<u32, TDescriptor>> loadingQueueBack;
		std::vector<std::pair<u32, TCreator>> createQueueFront;
		std::vector<std::pair<u32, TCreator>> createQueueBack;
		std::vector<u32> unloadingQueueFront;
		std::vector<u32> unloadingQueueBack;
		struct RessourceVersionPair {
			TRessource value;
			u16 version{ 0 };
			bool exists{ false };
		};
		std::vector<RessourceVersionPair> ressources;
	};
public:
	RenderRessourceManager() = default;
	RenderRessourceManager(RenderRessourceManager&&) = delete;
	RenderRessourceManager(const RenderRessourceManager&) = delete;
	RenderRessourceManager& operator=(RenderRessourceManager&&) = delete;
	RenderRessourceManager& operator=(const RenderRessourceManager&) = delete;

	THandle load(const TDescriptor& disc)
	{
		return getHandle(disc);
	}

	void unload(const TDescriptor& disc)
	{
		if (discToIndex.contains(disc)) {
			cu32 index = discToIndex[disc];
			discToIndex.erase(disc);
			freeRessourceSlots.push_back(index);
			ressourceSlots[index].exists = false;
			backend.queueUnload(index);
		}
	}

	THandle getHandle(const TDescriptor& disc)
	{
		THandle handle{};
		handle.managerId = MANAGER_ID;
		if (discToIndex.contains(disc)) {
			handle.index = discToIndex[disc];
		}
		else /* not loaded */{
			if (freeRessourceSlots.empty()) /* reuse old index */ {
				handle.index = nextRessourceIndex++;
				ressourceSlots.push_back({ 0, true });
			}
			else /* make new index */ {
				handle.index = freeRessourceSlots.back();
				freeRessourceSlots.pop_back();
				ressourceSlots[handle.index].version += 1;
				ressourceSlots[handle.index].exists = true;
			}
			discToIndex[disc] = handle.index;
			backend.queueLoad({ handle.index, disc });
		}
		handle.version = ressourceSlots[handle.index].version;
		ressourceSlots[handle.index].hasDescriptor = true;
		return handle;
	}

	bool contains(const TDescriptor& desc) const
	{
		return discToIndex.contains(desc);
	}

	THandle create(TCreator&& create, std::string const& name)
	{
		assert(!nameToIndex.contains(name));
		THandle handle;
		handle.managerId = MANAGER_ID;
		if (freeRessourceSlots.empty()) {
			handle.index = nextRessourceIndex++;
			ressourceSlots.push_back({ 0, true });
		}
		else {
			handle.index = freeRessourceSlots.back();
			freeRessourceSlots.pop_back();
			ressourceSlots[handle.index].version += 1;
			ressourceSlots[handle.index].exists = true;
		}
		backend.queueCreate({handle.index, std::move(create)});
		handle.version = ressourceSlots[handle.index].version;
		ressourceSlots[handle.index].hasDescriptor = false;
		nameToIndex.insert({ name,handle.index });
		return handle;
	}

	void destroy(const std::string& name)
	{
		if (nameToIndex.contains(name)) {
			const u32 index = nameToIndex[name];
			nameToIndex.erase(name);
			freeRessourceSlots.push_back(index);
			ressourceSlots[index].exists = false;
			backend.queueUnload(index);
		}
	}

	THandle getHandle(const std::string& name) const
	{
		assert(contains(name));
		u32 index = nameToIndex.at(name);
		THandle handle{};
		handle.index = index;
		handle.version = ressourceSlots[index].version;
		handle.managerId = MANAGER_ID;
		return handle;
	}

	bool contains(const std::string& name) const
	{
		return nameToIndex.contains(name);
	}

	bool isHandleValid(const THandle& handle) const
	{
		return
			handle.managerId == MANAGER_ID &&
			handle.index < nextRessourceIndex&&
			handle.version == ressourceSlots[handle.index].version &&
			ressourceSlots[handle.index].exists;
	}

	void clear()
	{
		for (u32 i = 0; i < ressourceSlots.size(); i++) {
			if (ressourceSlots[i].exists) {
				freeRessourceSlots.push_back(i);
				ressourceSlots[i].exists = false;
				backend.queueUnload(i);
			}
		}
		discToIndex.clear();
	}

	Backend* getBackend() { return &backend; }
protected:
	inline static u16 s_nextmanagerId{ 0 };
	const u16 MANAGER_ID{ s_nextmanagerId++ };
	robin_hood::unordered_map<TDescriptor, u32> discToIndex;
	robin_hood::unordered_map<std::string, u32> nameToIndex;
	u32 nextRessourceIndex{ 0 };
	struct Slot {
		u32 version{ 0 };
		bool hasDescriptor{ false };
		bool exists{ false };
	};
	std::vector<Slot> ressourceSlots;
	std::vector<u32> freeRessourceSlots;

	std::array<u8, 128> cachepadding0;
	Backend backend{ MANAGER_ID };
	std::array<u8, 128> cachepadding1;
};
