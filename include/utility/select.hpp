#ifndef __UTILITY_SELECT_HPP
#define __UTILITY_SELECT_HPP

/*
	根据char与wchar_t来选择使用类型

*/


namespace utility
{

	template < typename CharT, typename T, typename U >
	struct selector_t;

	template < typename T, typename U >
	struct selector_t<char, T, U>
	{
		typedef T type;
		type &t_;

		selector_t(T &&t, U &&)
			: t_(t)
		{}
	};

	template < typename T, typename U >
	struct selector_t<wchar_t, T, U>
	{
		typedef U type;
		type &t_;

		selector_t(T &&, U &&u)
			: t_(u)
		{}
	};

	template < typename CharT, typename T, typename U >
	inline typename selector_t<CharT, T, U>::type &select(T &&t, U &&u)
	{
		return selector_t<CharT, T, U>(t, u).t_;
	}


}


#endif