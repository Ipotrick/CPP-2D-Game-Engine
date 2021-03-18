#pragma once

#include "../JobSystem.hpp"
#include "../types/ShortNames.hpp"

#include "EntityComponentManagerView.hpp"

template<size_t REQUESTED_BATCH_SIZE, typename ComponentT>
JobSystem::Tag dispatchEntityWork(ComponentStoragePagedIndexing<ComponentT>& storage, std::function<void(EntityHandleIndex entity, ComponentT& comp)> func)
{
	constexpr size_t PAGE_BITS = ComponentStoragePagedIndexing<ComponentT>::PAGE_BITS;
	constexpr size_t PAGE_SIZE = ComponentStoragePagedIndexing<ComponentT>::PAGE_SIZE;

	class WorkerJob : public IJob {
	public:
		WorkerJob(ComponentStoragePagedIndexing<ComponentT>* storage, u32 beginPage, u32 endPage, std::function<void(EntityHandleIndex entity, ComponentT& comp)> const& func) :
			storage{ storage },
			beginPage{ beginPage },
			endPage{ endPage },
			func{ func }
		{}

		virtual void execute(const uint32_t threadId) override
		{
			for (u32 ip = beginPage; ip < endPage; ip++) {
				if (auto& page = storage->pages[ip]) {
					for (u32 i = 0; i < PAGE_SIZE; ++i) {
						u32 entIndex = (ip << PAGE_BITS) + i;
						if (storage->contains(entIndex)) {
							func(entIndex, page->data[i]);
						}
					}
				}
			}
		}
	private:
		ComponentStoragePagedIndexing<ComponentT>* storage{ nullptr };
		u32 beginPage{ 0xFFFFFFFF };
		u32 endPage{ 0xFFFFFFFF };
		std::function<void(EntityHandleIndex entity, ComponentT& comp)> func;
	};

	std::vector<WorkerJob> jobs;

	u32 beginPage{ 0 };
	u32 endPage{ 0 };
	u32 entitiesInCurrentBatch{ 0 };
	for (; endPage < storage.pages.size(); endPage++) {
		if (entitiesInCurrentBatch >= REQUESTED_BATCH_SIZE) {
			jobs.emplace_back(&storage, beginPage, endPage, func);
			beginPage = endPage;
			entitiesInCurrentBatch = 0;
		}
		if (storage.pages[endPage]) {
			entitiesInCurrentBatch += storage.pages[endPage]->usedCount;
		}
	}
	if (beginPage != endPage) {
		jobs.emplace_back(&storage, beginPage, endPage, func);
	}

	return JobSystem::submitVec(std::move(jobs));
}

template<size_t REQUESTED_BATCH_SIZE, typename ComponentT>
JobSystem::Tag dispatchEntityWork(ComponentStoragePagedSet<ComponentT>& storage, std::function<void(EntityHandleIndex entity, ComponentT& comp)> func) 
{

	class WorkerJob : public IJob {
	public:
		WorkerJob(ComponentStoragePagedSet<ComponentT>* storage, s32 begin, s32 end, std::function<void(EntityHandleIndex entity, ComponentT& comp)> const& func) :
			storage{ storage },
			begin{ begin },
			end{ end },
			func{ func }
		{}

		virtual void execute(const uint32_t threadId) override
		{
			for (auto iter = storage->begin() + begin; iter < storage->begin() + end; ++iter) {
				EntityHandleIndex entity = *iter;
				auto& data = iter.data();
				func(entity, data);
			}
		}
	private:
		ComponentStoragePagedSet<ComponentT>* storage{ nullptr };
		s32 begin{ -1 };
		s32 end{ -1 };
		std::function<void(EntityHandleIndex entity, ComponentT& comp)> func;
	};

	std::vector<WorkerJob> jobs;

	s32 beginOffset{ 0 };
	s32 endOffset{ 0 };
	for (; endOffset < storage.size(); endOffset += REQUESTED_BATCH_SIZE) {
		jobs.emplace_back(&storage, beginOffset, endOffset, func);
	}
	if (endOffset != storage.size()) {
		jobs.emplace_back(&storage, beginOffset, storage.size(), func);
	}

	return JobSystem::submitVec(std::move(jobs));
}
