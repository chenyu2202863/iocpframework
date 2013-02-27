#include "option_set.hpp"

#include "option_set.hpp"


namespace urdl 
{

	option_set::option_set()
	{
	}

	option_set::option_set(const option_set& other)
	{
		std::auto_ptr<option_wrapper_base>* prev_link = &head_;
		option_wrapper_base* node = other.head_.get();

		while (node)
		{
			prev_link->reset(node->clone());
			prev_link = &prev_link->get()->next;
			node = node->next.get();
		}
	}

	option_set::~option_set()
	{
	}

	option_set& option_set::operator=(const option_set& other)
	{
		option_set tmp(other);
		head_.reset(tmp.head_.release());
		return *this;
	}

	void option_set::set_options(const option_set& other)
	{
		option_wrapper_base* node = other.head_.get();
		while (node)
		{
			_set_option_wrapper_base(node->clone());
			node = node->next.get();
		}
	}

	void option_set::_set_option_wrapper_base(option_set::option_wrapper_base* o)
	{
		std::auto_ptr<option_wrapper_base>* prev_link = &head_;
		option_wrapper_base* node = head_.get();
		while (node)
		{
			if (o->type_info() == node->type_info())
			{
				o->next.reset(node->next.release());
				prev_link->reset(o);
				return;
			}
			prev_link = &node->next;
			node = node->next.get();
		}
		prev_link->reset(o);
	}

	option_set::option_wrapper_base* option_set::_get_option_wrapper_base(const std::type_info& ti) const
	{
		option_wrapper_base* node = head_.get();

		while (node)
		{
			if (ti == node->type_info())
				return node;
			node = node->next.get();
		}
		return 0;
	}

	void option_set::_clear_option_wrapper_base(const std::type_info& ti)
	{
		std::auto_ptr<option_wrapper_base>* prev_link = &head_;
		option_wrapper_base* node = head_.get();

		while (node)
		{
			if (ti == node->type_info())
			{
				prev_link->reset(node->next.release());
				return;
			}
			prev_link = &node->next;
			node = node->next.get();
		}
	}

}

