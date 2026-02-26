#pragma once

#include "SComponent.h"
#include <string>

namespace GX
{
	class SCSGModel : public SComponent
	{
	public:
		SCSGModel() {}
		~SCSGModel() {}

		virtual int getVersion() { return 0; }

		virtual void serialize(Serializer* s)
		{
			SComponent::serialize(s);
		}
	};
}