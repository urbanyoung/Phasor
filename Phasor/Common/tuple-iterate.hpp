#include <tuple>

namespace TupleHelpers {
	template<std::size_t I, typename... Tp>
	struct forward_comparator
	{
		static const bool value = (I < sizeof...(Tp));
		static const std::size_t start = 0;
		static const std::size_t next = I + 1;
	};

	template<std::size_t I, typename... Tp>
	struct reverse_comparator
	{
		static const std::size_t start = sizeof...(Tp)-1;
		static const std::size_t next = I - 1;
		static const bool value = I >= 0 && I < sizeof...(Tp);
	};

	template<
		template<std::size_t, typename...>class Comparator,
		std::size_t I,
	class F,
		typename... Tp
	>
	inline typename std::enable_if<!Comparator<I, Tp...>::value>::type
	citerate(const std::tuple<Tp...>&, F&)
	{ }

	template<
		template<std::size_t, typename...>class Comparator,
		std::size_t I,
	class F,
		typename... Tp
	>
	inline typename std::enable_if<Comparator<I, Tp...>::value>::type
	citerate(const std::tuple<Tp...>& t, F& f)
	{
		typedef Comparator<I, Tp...> comparator;
		f(std::get<I>(t));
		citerate<Comparator, comparator::next, F, Tp...>(t, f);
	}

	// Helper for specifying start value
	template<
		template<std::size_t, typename...>class Comparator,
	class F,
		typename... Tp
	>
	inline void citerate(const std::tuple<Tp...>& t, F& f)
	{
		citerate<Comparator, Comparator<0, Tp...>::start, F, Tp...>(t, f);
	}

	// ---------------------------------------------------
	// non-const versions
	template<
		template<std::size_t, typename...>class Comparator,
		std::size_t I,
	class F,
		typename... Tp
	>
	inline typename std::enable_if<!Comparator<I, Tp...>::value>::type
	iterate(std::tuple<Tp...>&, F&)
	{ }

	template<
		template<std::size_t, typename...>class Comparator,
		std::size_t I,
	class F,
		typename... Tp
	>
	inline typename std::enable_if<Comparator<I, Tp...>::value>::type
	iterate(std::tuple<Tp...>& t, F& f)
	{
		typedef Comparator<I, Tp...> comparator;
		f(std::get<I>(t));
		iterate<Comparator, comparator::next, F, Tp...>(t, f);
	}

	// Helper for specifying start value
	template<
		template<std::size_t, typename...>class Comparator,
	class F,
		typename... Tp
	>
	inline void iterate(std::tuple<Tp...>& t, F& f)
	{
		iterate<Comparator, Comparator<0, Tp...>::start, F, Tp...>(t, f);
	}
};