#pragma once
#include <vector>

class IndexSet {
	std::vector<bool> setTable;
	std::vector<int> elementList;
public:
	void resizeIfSmaller(size_t size) {
		if (setTable.size() < size) {
			setTable.reserve(size);
			setTable.resize(size, false);
		}
	}
	void emplace(int el) {
		setTable[el] = true;
		elementList.push_back(el);
	}
	void erase(int el) {
		if (el < setTable.size()) {
			if (setTable[el]) {
				setTable[el] = false;
				auto iter = elementList.begin();
				for (; iter != elementList.end(); ++iter)
					if (*iter == el) break;
				elementList.erase(iter);
			}
		}
	}
	bool contains(int el) {
		return setTable[el];
	}
	void clear() {
		for (auto el : elementList) {
			setTable[el] = false;
		}
		elementList.clear();
	}
	bool empty() {
		return elementList.size() == 0;
	}
	std::vector<int>::iterator begin() {
		return elementList.begin();
	}
	std::vector<int>::iterator end() {
		return elementList.end();
	}
};