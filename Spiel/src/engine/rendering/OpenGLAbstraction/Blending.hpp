#pragma once

#include "../../types/ShortNames.hpp"

namespace gl {
	enum class BlendingFactor : u32 {
		Zero = 0,
		One = 1,
		SrcColor = 0x0300,
		OneMinusSrcColor = 0x0301,
		SrcAlpha = 0x0302,
		OneMinusSrcAlpha = 0x0303,
		DstAlpha = 0x0304,
		OneMinusDstAlpha = 0x0305,
		DestColor = 0x0306,
		OneMinusDstColor = 0x0307,
		SrcAlphaSaturate = 0x0308
	};

	enum class BlendingEquasion : u32 {
		Add = 0x8006,
		Min = 0x8007,
		Max = 0x8008,
		Substract = 0x800A,
		ReverseSustract = 0x800B
	};

	struct BlendingFunction {
		BlendingFactor srcFactor{ BlendingFactor::SrcAlpha };
		BlendingFactor dstFactor{ BlendingFactor::OneMinusSrcAlpha };
		BlendingEquasion equasion{ BlendingEquasion::Add };
	};

	void enableBlending();

	void setBlendingEquasion(BlendingEquasion eq);

	void setBlendingFactors(BlendingFactor sourceFactor, BlendingFactor destinationFactor);

	void setBlending(BlendingFunction setting);
}