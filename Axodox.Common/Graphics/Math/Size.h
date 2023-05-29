#pragma once
#include "pch.h"

namespace Axodox::Graphics
{
	struct Size
	{
		int32_t Width, Height;

		float AspectRatio() const;

		static const Size Zero;

		Size Half() const;
	};
}