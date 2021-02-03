#pragma once

#include <vector>
#include <mutex>
#include <cassert>
#include <ranges>
#include <concepts>

#include <robin_hood.h>

#include "../../types/ShortNames.hpp"

struct RessourceHandleBase {
	u32 index{ 0xFFFFFFFF };
	u16 version{ 0 };
	u16 managerid{ 0xFFFF };
};

// TODO make concepts for THandle, TDescriptor, TRessource

template<
	typename THandle, 
	typename TDescriptor, 
	typename TRessource
>
class RenderRessourceManager {
protected:
	class Backend {
	public:
		// Frontend Interface:

		void queueLoad(std::tuple<u32, TDescriptor>&& res)
		{
			std::unique_lock l(mut);
			if (std::ranges::find(loadingQueue, res) == std::end(loadingQueue)) {
				loadingQueue.push_back(std::move(res));
			}
		}

		void queueUnload(u32 index)
		{
			std::unique_lock l(mut);
			if (std::ranges::find(unloadingQueue, index) == std::end(unloadingQueue)) {
				unloadingQueue.push_back(index);
			}
		}

		/**
		 * pushes data from frontend buffer to the backend.
		 * clears the frontend buffer afterwards.
		 */
		void push()
		{
			std::unique_lock l(mut);
			loadingQueueBack.insert(std::end(loadingQueueBack), std::begin(loadingQueue), std::end(loadingQueue));
			loadingQueue.clear();
			unloadingQueueBack.insert(std::end(unloadingQueueBack), std::begin(unloadingQueue), std::end(unloadingQueue));
			unloadingQueue.clear();
		}

		// Backend Interface:
		std::unique_lock<std::mutex> makeLock() const 
		{ 
			return std::unique_lock<std::mutex>(mut); 
		}

		/**
		 * call BEFORE the draw
		 */
		void loadQueue(std::unique_lock<std::mutex>& lock)
		{
			assert(lock.mutex() == mut && lock.owns_lock());
			for (auto& [index, discriptor] : loadingQueueBack) {
				if (index >= ressources.size()) {
					ressources.resize(index + 1);
				}
				ressources[index] = std::move(TRessource{ discriptor });
			}
			loadingQueueBack.clear();
		}

		/**
		 * call AFTER the draw
		 */
		void unloadQueue(std::unique_lock<std::mutex>& lock)
		{
			assert(lock.mutex() == mut && lock.owns_lock());
			for (auto& [index] : unloadingQueueBack) {
				ressources[index].reset();
			}
			unloadingQueueBack.clear();
		}

		bool isLoaded(std::unique_lock<std::mutex>& lock, s32 index) const
		{
			assert(lock.mutex() == mut && lock.owns_lock());
			return ressources[index].loaded();
		}

		void reload(std::unique_lock<std::mutex>& lock, s32 index) const
		{
			assert(lock.mutex() == mut && lock.owns_lock());
			ressources[index].reload();
		}

		TRessource& get(std::unique_lock<std::mutex>& lock, s32 index)
		{
			assert(lock.mutex() == mut && lock.owns_lock());
			ressources[index];
		}
	private:
		mutable std::mutex mut; 
		std::vector<std::tuple<u32, TDescriptor>> loadingQueue;
		std::vector<std::tuple<u32, TDescriptor>> loadingQueueBack;
		std::vector<u32> unloadingQueue;
		std::vector<u32> unloadingQueueBack;
		std::vector<TRessource> ressources;
	};
public:
	RenderRessourceManager() = default;
	RenderRessourceManager(RenderRessourceManager&&) = delete;
	RenderRessourceManager(const RenderRessourceManager&) = delete;
	RenderRessourceManager& operator=(RenderRessourceManager&&) = delete;
	RenderRessourceManager& operator=(const RenderRessourceManager&) = delete;

	void load(const TDescriptor& disc)
	{
		auto dummy = makeHandle(disc);
	}

	void unload(const TDescriptor& disc)
	{
		if (discToIndex.contains(disc)) {
			cu32 index = discToIndex[disc];
			discToIndex.erase(disc);
			freeRessourceSlots.push_back(index);
			backend.queueUnload(index);
		}
	}

	void clear()
	{
		for (auto& disc : discToIndex) {
			cu32 index = discToIndex[disc];
			freeRessourceSlots.push_back(index);
			backend.queueUnload(index);
		}
		discToIndex.clear();
		nextRessourceIndex = 0;
		freeRessourceSlots.clear();
		ressourceSlotVersions.clear();
	}

	THandle makeHandle(const TDescriptor& disc)
	{
		THandle handle;
		handle.managerid = MANAGER_ID;
		if (discToIndex.contains(disc)) {
			handle.index = discToIndex[disc];
		}
		else /* not loaded */{
			if (freeRessourceSlots.empty()) /* reuse old index */ {
				handle.index = nextRessourceIndex++;
				ressourceSlotVersions.push_back(0);
			}
			else /* make new index */ {
				handle.index = freeRessourceSlots.back();
				freeRessourceSlots.pop_back();
				ressourceSlotVersions[handle.index] += 1;
			}
			discToIndex[disc] = handle.index;
			backend.queueLoad({ handle.index, handle.version, disc });
		}
		handle.version = ressourceSlotVersions[handle.index];
		return handle;
	}

	bool isHandleValid(const THandle& handle) const
	{
		return
			handle.managerId == MANAGER_ID &&
			handle.index < nextRessourceIndex &&
			handle.version == ressourceSlotVersions[handle.index];
	}

	bool isLoaded(const TDescriptor& desc) const
	{
		return discToIndex.contains(desc);
	}

	Backend* getBackend() { return backend; }
protected:
	inline static u16 s_nextManagerId{ 0 };
	const u16 MANAGER_ID{ s_nextManagerId++ };
	robin_hood::unordered_map<TDescriptor, u32> discToIndex;
	u32 nextRessourceIndex{ 0 };
	std::vector<u16> ressourceSlotVersions;
	std::vector<u32> freeRessourceSlots;

	std::array<u8, 128> cachepadding0;
	Backend backend;
	std::array<u8, 128> cachepadding1;
};
