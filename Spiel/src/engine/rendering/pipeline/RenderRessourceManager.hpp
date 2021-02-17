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

template<
	typename THandle, 
	typename TDescriptor, 
	typename TRessource
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
		void loadQueue(std::unique_lock<std::mutex>& lock)
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

		const TRessource& get(std::unique_lock<std::mutex>& lock, s32 index)
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
			ressourceSlots[index].exists = false;
			backend.queueUnload(index);
		}
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

	THandle makeHandle(const TDescriptor& disc)
	{
		THandle handle;
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
		return handle;
	}

	bool isHandleValid(const THandle& handle) const
	{
		return
			handle.managerId == MANAGER_ID &&
			handle.index < nextRessourceIndex &&
			handle.version == ressourceSlots[handle.index].version &&
			ressourceSlots[handle.index].exists;
	}

	bool contains(const TDescriptor& desc) const
	{
		return discToIndex.contains(desc);
	}

	Backend* getBackend() { return &backend; }
protected:
	inline static u16 s_nextmanagerId{ 0 };
	const u16 MANAGER_ID{ s_nextmanagerId++ };
	robin_hood::unordered_map<TDescriptor, u32> discToIndex;
	u32 nextRessourceIndex{ 0 };
	struct Slot {
		u32 version{ 0 };
		bool exists{ false };
	};
	std::vector<Slot> ressourceSlots;
	std::vector<u32> freeRessourceSlots;

	std::array<u8, 128> cachepadding0;
	Backend backend{ MANAGER_ID };
	std::array<u8, 128> cachepadding1;
};
