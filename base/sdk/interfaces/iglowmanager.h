#pragma once
// used: color
#include "../datatypes/color.h"

#define END_OF_FREE_LIST -1
#define ENTRY_IN_USE -2

enum EGlowRenderStyle : int
{
	GLOWRENDERSTYLE_DEFAULT = 0,
	GLOWRENDERSTYLE_RIMGLOW3D,
	GLOWRENDERSTYLE_EDGE_HIGHLIGHT,
	GLOWRENDERSTYLE_EDGE_HIGHLIGHT_PULSE,
	GLOWRENDERSTYLE_COUNT,
};

class IGlowObjectManager
{
public:
	struct GlowObject_t
	{
		void Set(Color color, int nRenderStyle = GLOWRENDERSTYLE_DEFAULT) // @note: styles not used cuz other styles doesnt have ignorez flag and needed to rebuild glow
		{
			this->vecColor = Vector(color.rBase(), color.gBase(), color.bBase());
			this->flAlpha = color.aBase();
			this->flBloomAmount = 1.0f;
			this->bRenderWhenOccluded = true;
			this->bRenderWhenUnoccluded = false;
			this->nRenderStyle = nRenderStyle;
		}

		void Clear()
		{
			this->vecColor = {0, 0, 0};
			this->flAlpha = 0;
			this->flBloomAmount = 0.f;
			this->bRenderWhenOccluded = false;
			this->bRenderWhenUnoccluded = false;
			this->nRenderStyle = 0;
		}

		inline bool IsEmpty() const
		{
			return nNextFreeSlot != ENTRY_IN_USE;
		}

		CBaseEntity*	pEntity;
		Vector			vecColor;
		float			flAlpha;
		std::byte		pad0[0x8];
		float			flBloomAmount;
		float			flLocalPlayerIsZeroPoint;
		bool			bRenderWhenOccluded;
		bool			bRenderWhenUnoccluded;
		bool			bFullBloomRender;
		std::byte		pad1[0x1];
		int				iFullBloomStencilTestValue;
		int				nRenderStyle;
		int				nSplitScreenSlot;
		int				nNextFreeSlot;
	};

	CUtlVector<GlowObject_t> vecGlowObjectDefinitions;
	int nFirstFreeSlot;
};
