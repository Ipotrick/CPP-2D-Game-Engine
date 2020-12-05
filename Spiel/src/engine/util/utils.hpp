#pragma once


#include <tuple>
#include <functional>

namespace util {
	template<int s, int e>
	struct range {
		auto begin() {
			return iterator(s);
		}
		auto end() {
			if constexpr (s < e) {
				return iterator(e + 1);
			}
			else {
				return iterator(e - 1);
			}
		}
		class iterator {
			int current;
		public:
			iterator(int start) : current{ start } {  }
			typedef iterator self_type;
			typedef int value_type;
			typedef int& reference;
			typedef std::forward_iterator_tag iterator_category;
			self_type operator++(int junk) {
				if constexpr (s < e) {
					return current++;
				}
				else {
					return current--;
				}
			}
			self_type operator++() {
				auto temp = current;
				operator++(1);
				return temp;
			}
			reference operator*() {
				return current;
			}
			bool operator==(const self_type& rhs) {
				return current == rhs.current;
			}
			bool operator!=(const self_type& rhs) {
				return current != rhs.current;
			}
		};
	};

	template <
		size_t Index = 0, // start iteration at 0 index
		typename TTuple,  // the tuple type
		size_t Size =
		std::tuple_size_v<
		std::remove_reference_t<TTuple>>, // tuple size
		typename TCallable, // the callable to bo invoked for each tuple item
		typename... TArgs   // other arguments to be passed to the callable 
		>
		void tuple_for_each(TTuple&& tuple, TCallable&& callable, TArgs&&... args)
	{
		if constexpr (Index < Size)
		{
			std::invoke(callable, args..., std::get<Index>(tuple));

			if constexpr (Index + 1 < Size)
				tuple_for_each<Index + 1>(
					std::forward<TTuple>(tuple),
					std::forward<TCallable>(callable),
					std::forward<TArgs>(args)...);
		}
	}
}