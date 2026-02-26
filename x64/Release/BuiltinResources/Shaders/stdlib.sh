#ifndef __STDLIB_SH__
#define __STDLIB_SH__

#include "common.sh"

SAMPLER2D(u_deferredDepth, 9);
uniform vec4 u_transparentPass;

void discardGBufferOverlap(vec4 v_worldpos)
{
	if (u_transparentPass.x == 1) //Discard if deferred fragment is on top of this
	{
		float depth = (v_worldpos.z / v_worldpos.w) * 0.5 + 0.5;
		vec2 d_texcoord = (v_worldpos.xy/v_worldpos.w) * 0.5 + 0.5;
		float deferredDepth = texture2D(u_deferredDepth, d_texcoord).r;
		if (deferredDepth < depth)
			discard;
	}
}
#endif