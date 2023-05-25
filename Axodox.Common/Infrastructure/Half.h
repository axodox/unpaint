#pragma once
#include "pch.h"

namespace Axodox::Infrastructure
{
	class half
	{
	public:
		void operator=(float value);
		operator float() const;

	private:
		uint16_t _value;
	};
}