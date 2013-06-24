#ifndef __ASYNC_SERVICE_READ_WRITE_BUFFER_HPP
#define __ASYNC_SERVICE_READ_WRITE_BUFFER_HPP


#include <vector>
#include <string>
#include <array>
#include <type_traits>
#include <list>
#include <cassert>

#include "exception.hpp"
#include "../../extend_stl/allocator/stack_allocator.hpp"

namespace async { namespace service {

	// ---------------------------------------------------------
	// class mutable_buffer

	// 提供安全的修改，由外界提供缓冲区
	class mutable_buffer_t
	{
	public:
		typedef char					value_type;
		typedef char *					pointer;
		typedef const char*				const_pointer;
		typedef value_type*				iterator;
		typedef const value_type*		const_iterator;

	public:
		value_type *data_;
		size_t size_;

	public:
		mutable_buffer_t()
			: data_(0)
			, size_(0)
		{}
		mutable_buffer_t(value_type *data, size_t size)
			: data_(data)
			, size_(size)
		{}

		mutable_buffer_t(mutable_buffer_t &&rhs)
			: data_(rhs.data_)
			, size_(rhs.size_)
		{
		}

		~mutable_buffer_t()
		{

		}

	private:
		mutable_buffer_t(const mutable_buffer_t &);
		mutable_buffer_t &operator=(const mutable_buffer_t &);

	public:
		pointer data()
		{
			return data_;
		}

		size_t size() const
		{
			return size_;
		}

		iterator begin()
		{
			return data_;
		}
		iterator end()
		{
			return data_ + size_;
		}

		const_iterator begin() const
		{
			return data_;
		}
		const_iterator end() const
		{
			return data_ + size_;
		}
	};

	inline mutable_buffer_t operator+(const mutable_buffer_t &buffer, size_t sz)
	{
		assert(sz <= buffer.size_);
		if( sz > buffer.size_ )
			throw service::network_exception("sz > buffer.size_");

		mutable_buffer_t::pointer newData = buffer.data_ + sz;
		size_t newSize = buffer.size_ - sz;

		return mutable_buffer_t(newData, newSize);
	}
	inline mutable_buffer_t operator+(size_t sz, const mutable_buffer_t &buffer)
	{
		return buffer + sz;
	}


	// ---------------------------------------------------------
	// class ConstBuffer

	// 不能修改缓冲区，由外界提供缓冲区
	class const_buffer_t
	{
	public:
		typedef char					value_type;
		typedef char *					pointer;
		typedef const char*				const_pointer;
		typedef value_type*				iterator;
		typedef const value_type*		const_iterator;

	public:
		const value_type *data_;
		size_t size_;

	public:
		const_buffer_t()
			: data_(0)
			, size_(0)
		{}
		const_buffer_t(const value_type *data, size_t size)
			: data_(data)
			, size_(size)
		{}
		const_buffer_t(const const_buffer_t &rhs)
			: data_(rhs.data_)
			, size_(rhs.size_)
		{
		}
		const_buffer_t &operator=(const const_buffer_t &rhs)
		{
			if( &rhs != this )
			{
				data_ = rhs.data_;
				size_ = rhs.size_;
			}

			return *this;
		}

	public:
		const_pointer data() const
		{
			return data_;
		}

		size_t size() const
		{
			return size_;
		}


		const_iterator begin() const
		{
			return data_;
		}
		const_iterator end() const
		{
			return data_ + size_;
		}
	};


	inline const_buffer_t operator+(const const_buffer_t &buffer, size_t sz)
	{
		assert(sz <= buffer.size_);
		if( sz > buffer.size_ )
			throw service::network_exception("sz > buffer.size_");

		const_buffer_t::const_pointer newData = buffer.data_  + sz;
		size_t newSize = buffer.size_ - sz;

		return const_buffer_t(newData, newSize);
	}
	inline const_buffer_t operator+(size_t sz, const const_buffer_t &buffer)
	{
		return buffer + sz;
	}

	// ---------------------------------------------------------
	// class const_array_buffer_t

	struct const_array_buffer_t
	{
		enum { MAX_BUFFER_SIZE = 4 * 1024 };
		typedef stdex::allocator::stack_allocator_t<const_buffer_t, MAX_BUFFER_SIZE> const_buffer_allocator_t;
		typedef std::list<const_buffer_t, const_buffer_allocator_t> const_buffers_t;

		stdex::allocator::stack_storage_t<MAX_BUFFER_SIZE> storage_;
		const_buffer_allocator_t alloc_;
		const_buffers_t buffers_;
		std::uint16_t buffer_cnt_;
		std::uint32_t buffer_size_;

	public:
		const_array_buffer_t()
			: alloc_(&storage_)
			, buffers_(alloc_)
			, buffer_cnt_(0)
			, buffer_size_(0)
		{}
		~const_array_buffer_t()
		{}

		const_array_buffer_t(const_array_buffer_t &&rhs)
			: storage_(std::move(rhs.storage_))
			, alloc_(&storage_)
			, buffers_(rhs.buffers_, alloc_)
			, buffer_cnt_(rhs.buffer_cnt_)
			, buffer_size_(rhs.buffer_size_)
		{
		}

	private:
		const_array_buffer_t(const const_array_buffer_t &rhs);
		const_array_buffer_t &operator=(const const_array_buffer_t &rhs);

	public:
		std::uint16_t buffer_count() const
		{
			return buffer_cnt_;
		}

		std::uint32_t size() const
		{
			return buffer_size_;
		}

		const const_array_buffer_t &data() const
		{
			return *this;
		}

		const_array_buffer_t &add(const_buffer_t &&buffer)
		{
			buffer_cnt_ += 1;
			buffer_size_ += buffer.size();

			buffers_.push_front(std::move(buffer));
			return *this;
		}

		const_array_buffer_t &add(const_array_buffer_t &&buffer)
		{
			buffer_cnt_ += buffer.buffer_count();
			buffer_size_ += buffer.size();
			buffers_.splice(buffers_.begin(), std::move(buffer.buffers_));
			
			return *this;
		}
	};

	inline const_array_buffer_t &operator<<(const_array_buffer_t &array_buffer, const_buffer_t &&buffer)
	{
		array_buffer.add(std::move(buffer));
		return array_buffer;
	}

	inline const_array_buffer_t &operator<<(const_array_buffer_t &array_buffer, const_array_buffer_t &&buffer)
	{
		array_buffer.add(std::move(buffer));
		return array_buffer;
	}

	inline const_array_buffer_t operator+(const const_array_buffer_t &const_buffer, std::uint32_t sz)
	{
		const_array_buffer_t &buffer = const_cast<const_array_buffer_t &>(const_buffer);
		for(auto iter = buffer.buffers_.begin(); iter != buffer.buffers_.end(); ++iter)
		{
			if( sz >= iter->size() )
			{
				sz -= iter->size();
				continue;
			}
			else
			{
				auto this_mutable_buf = *iter + sz;
				const_array_buffer_t tmp;
				tmp.add(std::move(this_mutable_buf));
				std::for_each(++iter, buffer.buffers_.end(), 
					[&tmp](const_array_buffer_t::const_buffers_t::value_type &val)
				{
					tmp.buffers_.push_back(std::move(val));
				});

				return std::move(tmp);
			}
		}

		return std::move(buffer);
	}
	inline const_array_buffer_t operator+(std::uint32_t sz, const const_array_buffer_t &buffer)
	{
		return std::move(buffer + sz);
	}



	// ------------------------------------------------------
	// MutableBuffer helper

	inline mutable_buffer_t &buffer(mutable_buffer_t &buf)
	{
		return buf;
	}

	inline mutable_buffer_t buffer(char *data, std::uint32_t sz)
	{
		return mutable_buffer_t(data, sz);
	}

	template < typename PodT, std::uint32_t _N >
	inline mutable_buffer_t buffer(PodT (&data)[_N])
	{
		static_assert(std::is_pod<PodT>::value, "array template param must be PodT type");
		return mutable_buffer_t(data, _N * sizeof(PodT));
	}

	template < typename PodT, std::uint32_t _N >
	inline mutable_buffer_t buffer(std::array<PodT, _N> &data)
	{
		static_assert(std::is_pod<PodT>::value, "array template param must be PodT type");
		return mutable_buffer_t(data.data(), _N * sizeof(PodT));
	}

	template < typename PodT >
	inline mutable_buffer_t buffer(std::vector<PodT> &data)
	{
		return mutable_buffer_t(&data[0], data.size() * sizeof(PodT));
	}

	// ------------------------------------------------------
	// ConstBuffer helper

	inline const_buffer_t &buffer(const_buffer_t &buf)
	{
		return buf;
	}
	inline const_buffer_t buffer(const char *data, std::uint32_t sz)
	{
		return const_buffer_t(data, sz);
	}

	template<typename PodT, std::uint32_t _N>
	inline const_buffer_t buffer(const PodT (&data)[_N])
	{
		static_assert(std::is_pod<PodT>::value, "array template param must be PodT type");
		return const_buffer_t(data, _N * sizeof(PodT));
	}

	template<typename PodT, std::uint32_t _N>
	inline const_buffer_t buffer(const std::array<PodT, _N> &data)
	{
		static_assert(std::is_pod<PodT>::value, "array template param must be PodT type");
		return const_buffer_t(data.data(), _N * sizeof(PodT));
	}

	template<typename PodT>
	inline const_buffer_t buffer(const std::vector<PodT> &data)
	{
		return const_buffer_t(&data[0], data.size() * sizeof(PodT));
	}

	inline const_buffer_t buffer(const std::string &data)
	{
		return const_buffer_t(data.data(), data.length());
	}	

	inline const_array_buffer_t buffer(const const_array_buffer_t &buf, std::uint32_t sz)
	{
		return std::move(const_cast<const_array_buffer_t &>(buf));
	}
}
}




#endif