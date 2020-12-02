#pragma once

#include <vector>
#include <array>

template<typename T, size_t PAGE_SIZE>
class SparseBuffer {
	static_assert(ceil(log2(PAGE_SIZE)) == floor(log2(PAGE_SIZE)), "error: PAGE_SIZE must be a power of 2");

	constexpr size_t PAGE_BITS = log2(PAGE_SIZE);

	constexpr size_t OFFSET_MASK = ~(-1 << PAGE_BITS);

	size_t page(size_t index) const {
		return index >> PAGE_BITS;
	}

	size_t offset(size_t offset) const {
		return index & OFFSET_MASK;
	}

	struct Page {
		std::array<T, PAGE_SIZE> data;
		size_t size{ 0 };
	};

public:
	SparseBuffer(size_t maxIndex) {
		if (pages.size() <= page(maxIndex)) {
			pages.resize(page(maxIndex) + 1);
		}
	}

	void write(size_t index, T&& value) {
		if (pages.size() <= page(maxIndex))
			pages.resize(page(maxIndex) + 1);
		if (pages.at(page(index)) == nullptr)
			pages.at(page(index)) = new Page();
		pages.at(page(index))->data.at(offset(index)) = value;
		pages.at(page(index))->size += 1;
	}

	const T& at(size_t index) const {
		return pages[page(index)]->data[offset(index)];
	}

	void clear() {
		for (auto& page : pages) {
			if (page) {
				for (auto& element : page->data) {
					data = T()
				}
				page->size = 0;
			}
		}
	}

	void shrinkToFit() {
		for (auto& page : pages) {
			if (page != nullptr && page->size == 0) {
				delete page;
				page = nullptr;
			}
		}
	}

	void reset() {
		for (auto& page : pages) {
			if (page != nullptr) {
				delete page;
				page = nullptr;
			}
		}
		pages.clear();
		pages.shrink_to_fit();
	}

private:
	std::vector<Page*> pages;
};