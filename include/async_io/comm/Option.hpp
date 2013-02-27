#ifndef __COMM_OPTION_HPP
#define __COMM_OPTION_HPP

#include "../Basic.hpp"

namespace async
{
	namespace comm
	{

		class BaudRate
		{
			unsigned int value_;

		public:
			explicit BaudRate(unsigned int rate = 0);
			unsigned int Value() const;
			void Store(DCB&) const;
			void Load(const DCB&);
		};


		class FlowControl
		{
		public:
			enum type { None, Software, Hardware };
			type value_;

			explicit FlowControl(type t = None);
			type Value() const;
			void Store(DCB&) const;
			void Load(const DCB&);
		};


		class Parity
		{
		public:
			enum type { None, Odd, Even };
			type value_;

			explicit Parity(type t = None);
			type Value() const;
			void Store(DCB&) const;
			void Load(const DCB&);
		};


		class StopBits
		{
		public:
			enum type { One, Onepointfive, Two };
			type value_;

			explicit StopBits(type t = One);
			type Value() const;
			void Store(DCB&) const;
			void Load(const DCB&);
		};


		class CharacterSize
		{
			unsigned int value_;

		public:
			explicit CharacterSize(unsigned int t = 8);
			unsigned int Value() const;
			void Store(DCB&) const;
			void Load(const DCB&);
		};
	}
}



#endif