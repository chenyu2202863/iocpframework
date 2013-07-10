#ifndef __UTILITY_CICULAR_BUFFER_HPP
#define __UTILITY_CICULAR_BUFFER_HPP

#include <array>
#include <cstdint>


namespace utility {

	template < typename T, std::uint32_t N >
	class cicular_buffer_t
	{
		std::array<T, N> buffer_;
		std::uint32_t index_;

	public:
		cicular_buffer_t()
			: index_(0)
		{}

	private:
		cicular_buffer_t(const cicular_buffer_t &);
		cicular_buffer_t &operator=(const cicular_buffer_t &);

	public:
		const T &back() const
		{
			return buffer_[index_ == 0 ? N - 1 : index_ - 1];
		}

		T &back()
		{
			return buffer_[index_ == 0 ? N - 1 : index_ - 1];
		}

		void push_back(T && val)
		{
			buffer_[index_++] = std::move(val);
			if ( index_ >= N )
				index_ = 0;
		}

	};
}

#endif