#ifndef __UTILITY_MOVE_WRAPPER_HPP
#define __UTILITY_MOVE_WRAPPER_HPP


namespace utility {

	template < typename T >
	struct move_on_copy_wrapper_t
	{
		mutable T value_;

		move_on_copy_wrapper_t(T&& t)
			: value_(std::move(t))
		{}

		move_on_copy_wrapper_t(const move_on_copy_wrapper_t &other)
			: value_(std::move(other.value_))
		{}

		move_on_copy_wrapper_t(move_on_copy_wrapper_t&& other)
			: value_(std::move(other.value_))
		{}

		move_on_copy_wrapper_t& operator=(const move_on_copy_wrapper_t &other)
		{
			if( this != &other )
			{
				value_ = std::move(other.value_);
			}

			return *this;
		}

		move_on_copy_wrapper_t& operator=(move_on_copy_wrapper_t &&other)
		{
			if( this != &other )
			{
				value_ = std::move(other.value_);
			}
			
			return *this;
		}

		operator T &()
		{
			return value_;
		}

		operator const T &() const
		{
			return value_;
		}
	};

	template < typename T >
	move_on_copy_wrapper_t<T> make_move_obj(T &&val)
	{
		return move_on_copy_wrapper_t<T>(std::forward<T>(val));
	}
}

#endif