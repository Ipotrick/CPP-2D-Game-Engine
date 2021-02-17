#pragma once

#include <optional>

#include <robin_hood.h>

#include "RessourceHandle.hpp"

template<
	typename THandle,
	typename TDescriptor,
	typename TRessource
>
class RessourceManager {
public:
	RessourceManager() = default;
	RessourceManager(RessourceManager&&) = delete;
	RessourceManager(const RessourceManager&) = delete;
	RessourceManager& operator=(RessourceManager&&) = delete;
	RessourceManager& operator=(const RessourceManager&) = delete;

	void load(const TDescriptor& desc)
	{
		assert(!descToIndex.contains(desc));
		makeHandle(desc);
	}

	void unload(const TDescriptor& desc)
	{
		assert(descToIndex.contains(desc));
		cu32 index = descToIndex[desc];
		descToIndex.erase(desc);
		unload(index);
	}

	void clear()
	{
		for (u32 i = 0; i < ressourceSlots.size(); i++) {
			if (ressourceSlots[i].ressource.has_value()) {
				unload(i);
			}
		}
		descToIndex.clear();
	}

	THandle makeHandle(const TDescriptor& desc)
	{
		THandle handle;
		handle.managerId = MANAGER_ID;
		if (descToIndex.contains(desc)) {
			handle.index = descToIndex[desc];
		}
		else /* not loaded */ {
			if (freeRessourceSlots.empty()) /* reuse old index */ {
				handle.index = nextRessourceIndex++;
				ressourceSlots.push_back(Slot{ .ressource={desc}, .version=0 });
			}
			else /* make new index */ {
				handle.index = freeRessourceSlots.back();
				freeRessourceSlots.pop_back();
				ressourceSlots[handle.index].version += 1;
				ressourceSlots[handle.index].ressource.emplace( desc );
			}
			descToIndex[desc] = handle.index;
		}
		handle.version = ressourceSlots[handle.index].version;
		return handle;
	}

	bool isHandleValid(const THandle& handle) const
	{
		return
			handle.managerId == MANAGER_ID &&
			handle.index < nextRessourceIndex&&
			handle.version == ressourceSlots[handle.index].version &&
			ressourceSlots[handle.index].ressource.has_value();
	}

	bool contains(const TDescriptor& desc) const
	{
		return descToIndex.contains(desc);
	}

	const TRessource& get(const THandle& handle) const
	{
		assert(isHandleValid(handle));
		return ressourceSlots[handle.index].ressource.value();
	}
protected:
	void unload(u32 index)
	{
		freeRessourceSlots.push_back(index);
		ressourceSlots[index].ressource.reset();
	}

	inline static u16 s_nextmanagerId{ 0 };
	const u16 MANAGER_ID{ s_nextmanagerId++ };
	robin_hood::unordered_map<TDescriptor, u32> descToIndex;
	u32 nextRessourceIndex{ 0 };
	struct Slot {
		std::optional<TRessource> ressource;
		u32 version{ 0 };
	};
	std::vector<Slot> ressourceSlots;
	std::vector<u32> freeRessourceSlots;
};

