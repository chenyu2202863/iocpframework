#include "Option.hpp"

#include <exception>


namespace async
{
	namespace comm
	{

		BaudRate::BaudRate(unsigned int rate /* = 0 */)
			: value_(rate)
		{

		}

		unsigned int BaudRate::Value() const
		{
			return value_;
		}

		void BaudRate::Store(DCB& dcb) const
		{
			dcb.BaudRate = value_;
		}

		void BaudRate::Load(const DCB& dcb)
		{
			value_ = dcb.BaudRate;
		}



		FlowControl::FlowControl(type t)
			: value_(t)
		{
			if (t != None && t != Software && t != Hardware)
				throw std::out_of_range("invalid FlowControl value");
		}

		FlowControl::type FlowControl::Value() const
		{
			return value_;
		}

		void FlowControl::Store(DCB& dcb) const
		{
			dcb.fOutxCtsFlow = FALSE;
			dcb.fOutxDsrFlow = FALSE;
			dcb.fTXContinueOnXoff = TRUE;
			dcb.fDtrControl = DTR_CONTROL_ENABLE;
			dcb.fDsrSensitivity = FALSE;
			dcb.fOutX = FALSE;
			dcb.fInX = FALSE;
			dcb.fRtsControl = RTS_CONTROL_ENABLE;
			switch (value_)
			{
			case None:
				break;
			case Software:
				dcb.fOutX = TRUE;
				dcb.fInX = TRUE;
				break;
			case Hardware:
				dcb.fOutxCtsFlow = TRUE;
				dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
				break;
			default:
				break;
			}
		}

		void FlowControl::Load(const DCB& dcb)
		{
			if( dcb.fOutX && dcb.fInX )
			{
				value_ = Software;
			}
			else if( dcb.fOutxCtsFlow && dcb.fRtsControl == RTS_CONTROL_HANDSHAKE )
			{
				value_ = Hardware;
			}
			else
			{
				value_ = None;
			}
		}


		Parity::Parity(type t)
			: value_(t)
		{
			if (t != None && t != Odd && t != Even)
				throw std::out_of_range("invalid Parity value");
		}

		Parity::type Parity::Value() const
		{
			return value_;
		}

		void Parity::Store(DCB& dcb) const
		{
			switch (value_)
			{
			case None:
				dcb.fParity = FALSE;
				dcb.Parity = NOPARITY;
				break;
			case Odd:
				dcb.fParity = TRUE;
				dcb.Parity = ODDPARITY;
				break;
			case Even:
				dcb.fParity = TRUE;
				dcb.Parity = EVENPARITY;
				break;
			default:
				break;
			}
		}

		void Parity::Load(const DCB& dcb)
		{
			if (dcb.Parity == EVENPARITY)
			{
				value_ = Even;
			}
			else if (dcb.Parity == ODDPARITY)
			{
				value_ = Odd;
			}
			else
			{
				value_ = None;
			}
		}


		StopBits::StopBits(type t)
			: value_(t)
		{
			if (t != One && t != Onepointfive && t != Two)
				throw std::out_of_range("invalid StopBits value");
		}

		StopBits::type StopBits::Value() const
		{
			return value_;
		}

		void StopBits::Store(DCB& dcb) const
		{
			switch (value_)
			{
			case One:
				dcb.StopBits = ONESTOPBIT;
				break;
			case Onepointfive:
				dcb.StopBits = ONE5STOPBITS;
				break;
			case Two:
				dcb.StopBits = TWOSTOPBITS;
				break;
			default:
				break;
			}
		}

		void StopBits::Load(const DCB& dcb)
		{
			if (dcb.StopBits == ONESTOPBIT)
			{
				value_ = One;
			}
			else if (dcb.StopBits == ONE5STOPBITS)
			{
				value_ = Onepointfive;
			}
			else if (dcb.StopBits == TWOSTOPBITS)
			{
				value_ = Two;
			}
			else
			{
				value_ = One;
			}
		}


		CharacterSize::CharacterSize(unsigned int t)
			: value_(t)
		{
			if (t < 5 || t > 8)
				throw std::out_of_range("invalid CharacterSize value");
		}

		unsigned int CharacterSize::Value() const
		{
			return value_;
		}

		void CharacterSize::Store(DCB& dcb) const
		{
			dcb.ByteSize = value_;
		}

		void CharacterSize::Load(const DCB& dcb)
		{
			value_ = dcb.ByteSize;
		}
	}
}