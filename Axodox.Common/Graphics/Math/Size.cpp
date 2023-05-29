#include "pch.h"
#include "Size.h"

namespace Axodox::Graphics
{
	const Size Size::Zero = { 0, 0 };

	float Size::AspectRatio() const
	{
		return float(Width) / float(Height);
	}

	Size Size::Half() const
	{
		return { Width / 2, Height / 2 };
	}
}