#ifndef __ASYNC_URDL_OPTION_SET_HPP
#define __ASYNC_URDL_OPTION_SET_HPP

#include <memory>
#include <typeinfo>


namespace urdl 
{

	class option_set
	{
	private:
		struct option_wrapper_base;
		std::auto_ptr<option_wrapper_base> head_;

	public:
		option_set();
		option_set(const option_set& other);
		~option_set();
		option_set& operator=(const option_set& other);

	public:
		template <typename Option>
		void set_option(const Option& o)
		{
			_set_option_wrapper_base(new option_wrapper<Option>(o));
		}

		void set_options(const option_set& other);


		template <typename Option>
		Option get_option() const
		{
			if( option_wrapper_base* o = _get_option_wrapper_base(typeid(option_wrapper<Option>)) )
				return static_cast<option_wrapper<Option>*>(o)->value;
			return Option();
		}


		template <typename Option>
		void clear_option()
		{
			_clear_option_wrapper_base(typeid(option_wrapper<Option>));
		}

	private:
		struct option_wrapper_base
		{
			virtual ~option_wrapper_base() 
			{}
			virtual const std::type_info& type_info() const = 0;
			virtual option_wrapper_base* clone() const = 0;

			std::auto_ptr<option_wrapper_base> next;
		};

		template < typename Option >
		struct option_wrapper 
			: option_wrapper_base
		{
			option_wrapper(const Option& o) 
				: value(o) 
			{}
			const std::type_info& type_info() const
			{ return typeid(option_wrapper<Option>); }

			option_wrapper_base* clone() const
			{ return new option_wrapper<Option>(value); }

			Option value;
		};

	public:
		void _set_option_wrapper_base(option_wrapper_base* o);
		option_wrapper_base* _get_option_wrapper_base(const std::type_info& ti) const;
		void _clear_option_wrapper_base(const std::type_info& ti);


	};

} 

#endif 
