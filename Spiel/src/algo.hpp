#pragma once

#include <tuple>
#include <functional>

template <
	size_t Index = 0, // start iteration at 0 index
	typename TTuple,  // the tuple type
	size_t Size =
	std::tuple_size_v<
	std::remove_reference_t<TTuple>>, // tuple size
	typename TCallable, // the callable to bo invoked for each tuple item
	typename... TArgs   // other arguments to be passed to the callable 
	>
	void for_each(TTuple&& tuple, TCallable&& callable, TArgs&&... args)
{
	if constexpr (Index < Size)
	{
		std::invoke(callable, args..., std::get<Index>(tuple));

		if constexpr (Index + 1 < Size)
			for_each<Index + 1>(
				std::forward<TTuple>(tuple),
				std::forward<TCallable>(callable),
				std::forward<TArgs>(args)...);
	}
}