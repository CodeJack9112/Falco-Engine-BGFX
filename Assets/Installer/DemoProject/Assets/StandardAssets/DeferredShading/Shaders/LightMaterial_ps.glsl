#define LIGHT_POINT         1
#define LIGHT_SPOT          2
#define LIGHT_DIRECTIONAL   3

#version 150

#if LIGHT_TYPE == LIGHT_DIRECTIONAL
in vec2 oUv0;
in vec3 oRay;
#else
in vec4 oPos;
#endif
    
uniform sampler2D Tex0;
uniform sampler2D Tex1;

#if LIGHT_TYPE != LIGHT_POINT
uniform vec3 lightDir;
#endif

#if LIGHT_TYPE == LIGHT_SPOT
uniform vec4 spotParams;
#endif

#if LIGHT_TYPE != LIGHT_DIRECTIONAL
uniform float vpWidth;
uniform float vpHeight;
uniform vec3 farCorner;
uniform float flip;
#endif

#ifdef IS_SHADOW_CASTER
uniform mat4 invView;
uniform mat4 shadowViewProjMat;
#if LIGHT_TYPE == LIGHT_POINT
uniform samplerCube ShadowTex;
#else
uniform sampler2D ShadowTex;
#endif
uniform vec3 shadowCamPos;
uniform float shadowFarClip;
#endif

uniform float farClipDistance;
uniform float nearClipDistance;

// Attributes of light
uniform vec4 lightDiffuseColor;
uniform vec4 lightSpecularColor;
uniform vec4 lightFalloff;
uniform vec4 lightPos;
uniform vec4 lightPosWorld;
uniform float lightPower;

out vec4 fragColour;

float checkShadow(
    sampler2D shadowMap,
    vec3 viewPos,
    mat4 invView,
    mat4 shadowViewProj,
    float shadowFarClip,
//#if LIGHT_TYPE == LIGHT_DIRECTIONAL
    vec3 shadowCamPos
//#else
//    float distanceFromLight
//#endif
    )
{
    vec3 worldPos = (invView * vec4(viewPos, 1)).xyz;
//#if LIGHT_TYPE == LIGHT_DIRECTIONAL
    float distanceFromLight = length(shadowCamPos-worldPos);
//#endif
    vec4 shadowProjPos = shadowViewProj * vec4(worldPos,1);
    shadowProjPos /= shadowProjPos.w;
    vec2 shadowSampleTexCoord = shadowProjPos.xy;
    float shadowDepth = texture(shadowMap, shadowSampleTexCoord).r;
    float shadowDistance = shadowDepth * shadowFarClip;
    if((shadowDistance - distanceFromLight + 0.5) < 0.0)
        return 0.0;
	else
		return 1.0;
}

void main()
{
    // None directional lights have some calculations to do in the beginning of the pixel shader
#if LIGHT_TYPE != LIGHT_DIRECTIONAL
    vec4 normProjPos = oPos / oPos.w;
    // -1 is because generally +Y is down for textures but up for the screen
    vec2 oUv0 = vec2(normProjPos.x, normProjPos.y * -1 * flip) * 0.5 + 0.5;
    vec3 oRay = vec3(normProjPos.x, normProjPos.y * flip, 1) * farCorner;
#endif
    
    vec4 a0 = texture(Tex0, oUv0); // Attribute 0: Diffuse color+shininess
    vec4 a1 = texture(Tex1, oUv0); // Attribute 1: Normal+depth

    // Attributes
    vec3 colour = a0.rgb;
    float specularity = a0.a;
    float distance = a1.w;  // Distance from viewer (w)
    vec3 normal = a1.xyz;

    // Calculate position of texel in view space
    vec3 viewPos = normalize(oRay)*distance*farClipDistance;
	float len_sq = 1.0;
	float len = 1.0;

    // Calculate light direction and distance
#if LIGHT_TYPE == LIGHT_DIRECTIONAL
    vec3 objToLightDir = -lightDir.xyz;
#else
    vec3 objToLightVec = lightPos.xyz - viewPos;
    len_sq = dot(objToLightVec, objToLightVec);
    len = sqrt(len_sq);
    vec3 objToLightDir = objToLightVec / len;
#endif

float final = 1.0;

#ifdef IS_SHADOW_CASTER
	float inverseShadowmapSize = 0.0009765625;
	float fixedDepthBias = 0.0005;
	float gradientClamp = 0.0098;
	float gradientScaleBias = 0.0;
	
	#if LIGHT_TYPE == LIGHT_DIRECTIONAL
	fixedDepthBias = 0.00115;
	#endif

	#if LIGHT_TYPE != LIGHT_POINT
	vec3 worldPos = (invView * vec4(viewPos, 1)).xyz;
	vec4 shadowProjPos = shadowViewProjMat * vec4(worldPos,1);
	shadowProjPos /= shadowProjPos.w;
	
	float pixeloffset = inverseShadowmapSize;
	
	vec3 shadowUV = shadowProjPos.xyz;
	vec4 shadowColor = texture2D(ShadowTex, shadowUV.xy);
	vec4 shadowColor1 = texture2D(ShadowTex, shadowUV.xy + vec2(-pixeloffset, 0));
	vec4 shadowColor2 =	texture2D(ShadowTex, shadowUV.xy + vec2(+pixeloffset, 0));
	vec4 shadowColor3 =	texture2D(ShadowTex, shadowUV.xy + vec2(0, -pixeloffset));
	vec4 shadowColor4 = texture2D(ShadowTex, shadowUV.xy + vec2(0, +pixeloffset));

	float centerdepth = shadowColor.x;
	
	vec4 depths = vec4(
		shadowColor1.x,
		shadowColor2.x,
		shadowColor3.x,
		shadowColor4.x);
		
	vec2 differences = abs( depths.yw - depths.xz );
	float gradient = min(gradientClamp, max(differences.x, differences.y));
	float gradientFactor = gradient * gradientScaleBias;
		
	float depthAdjust = gradientFactor + (fixedDepthBias * centerdepth);
	float finalCenterDepth = centerdepth + depthAdjust;

	depths += depthAdjust;
	final += (depths.x > shadowUV.z) ? 1.0 : 0.0;
	final += (depths.y > shadowUV.z) ? 1.0 : 0.0;
	final += (depths.z > shadowUV.z) ? 1.0 : 0.0;
	final += (depths.w > shadowUV.z) ? 1.0 : 0.0;
	
	final *= 0.2;
	
	#endif
	#if LIGHT_TYPE == LIGHT_POINT
	vec3 worldPos = (invView * vec4(viewPos, 1)).xyz;
	vec3 lightDir = (worldPos - shadowCamPos);

	float shadow  = 0.0;
	float bias    = 0.05; 
	float samples = 2.0;
	float offset  = 0.1;
	for(float x = -offset; x < offset; x += offset / (samples * 0.5))
	{
		for(float y = -offset; y < offset; y += offset / (samples * 0.5))
		{
			for(float z = -offset; z < offset; z += offset / (samples * 0.5))
			{
				vec4 shadowData = texture(ShadowTex, vec3(-lightDir.x, lightDir.y, lightDir.z) + vec3(x, y, z));
				float sampledDistance = shadowData.r;
				
				vec3 fromLightToFragment = lightPosWorld.xyz - worldPos;
				float distanceToLight = length(fromLightToFragment);
				float currentDistanceToLight = (distanceToLight - nearClipDistance) / (1000.0 - nearClipDistance);
				currentDistanceToLight = clamp(currentDistanceToLight, 0, 1);
	
				if(sampledDistance > currentDistanceToLight)
					shadow += 1.0;
			}
		}
	}
	shadow /= (samples * samples * samples);
	final = shadow;
	
	#endif
#endif
    
    // Calculate diffuse colour
    vec3 total_light_contrib;
    total_light_contrib = max(0.0,dot(objToLightDir, normal)) * lightDiffuseColor.rgb;

#if IS_SPECULAR
    // Calculate specular component
    vec3 viewDir = -normalize(viewPos);
    vec3 h = normalize(viewDir + objToLightDir);
    vec3 light_specular = pow(dot(normal, h),32.0) * lightSpecularColor.rgb;

    total_light_contrib += specularity * light_specular;
#endif

#if IS_ATTENUATED
    //if(lightFalloff.x - len < 0.0)
    //    discard;
    // Calculate attenuation
    //float attenuation = dot(lightFalloff.yzw, vec3(1.0, len, len_sq)) * lightPower;
    //total_light_contrib /= attenuation;
	
	//Compute attenuation value
	float att = 1.0;
	if (lightPos.w > 0.0)
	{
		att = smoothstep(lightFalloff.x, 0.0, len) * lightPower;
	}
	else
	{
		att = lightPower;
	}
	
	total_light_contrib *= att;
		
#endif

#if LIGHT_TYPE == LIGHT_SPOT
    float spotlightAngle = clamp(dot(lightDir.xyz, -objToLightDir), 0.0, 1.0);
    float spotFalloff = clamp((spotlightAngle - spotParams.x) / (spotParams.y - spotParams.x), 0.0, 1.0);
    total_light_contrib *= (1.0-spotFalloff);
#endif

    fragColour = vec4((total_light_contrib * colour) * final, 0.0);
    //fragColour = vec4(final, final, final, 0.0);
}
