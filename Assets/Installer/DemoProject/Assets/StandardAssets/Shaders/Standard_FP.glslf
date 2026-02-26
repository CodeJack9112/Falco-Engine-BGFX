uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D shadowMap;
uniform vec4 lightDiffuse;
uniform vec4 lightSpecular;
uniform vec4 lightPosition;
uniform vec4 lightAttenuation;
uniform vec4 spotlightParams;
uniform vec4 spotlightDir;
uniform vec4 lightParams;
uniform float lightPower;
uniform float shininess;
uniform vec4 ambient;
uniform int passNumber;
uniform mat4 modelMatrix;
uniform mat4 worldViewProj;
uniform float alphaCutoff;
uniform vec4 specularColor;
uniform vec3 eyePosition;

varying vec4 oUv0;
varying vec4 fragPos;
varying vec3 vNormal;
varying vec3 vTangent;
varying vec4 oUv;

float inverseShadowmapSize = 0.0007765625;
float fixedDepthBias = 0.001;
float gradientClamp = 0.0098;
float gradientScaleBias = 0.0;

vec3 expand(vec3 v)
{
	return (v - 0.5) * 2.0;
}

void main()
{
	vec4 diffuse = texture2D(diffuseMap, oUv0.xy);
	
	if (diffuse.a - alphaCutoff < 0.0) discard;
	
	vec3 lightVec;
	vec3 lightDir;
	float ndot = 0.0;
	float att = 1.0;
	float specFactor = 0.0;
	float spot = 1.0;

	if (passNumber == 1)
	{
		lightDir;
		vec3 oTSLightDir;
		vec3 oTSHalfAngle;
		mat3 rotation;
		vec3 eyeDir;
		vec3 bumpVec;
	
		// Calculate tangent space light vector
		// Get object space light direction
		lightDir = normalize((lightPosition - ((modelMatrix * fragPos) * lightPosition.w))).xyz;

		//Convert normals from object to world space
		vec3 normal = normalize((modelMatrix * vec4(vNormal, 0.0)).xyz);
		vec3 tangent = normalize((modelMatrix * vec4(vTangent, 0.0)).xyz);
		
		// Calculate the binormal (NB we assume both normal and tangent are already normalised)
		vec3 binormal = cross(normal, tangent);
		
		// Form a rotation matrix out of the vectors
		rotation = mat3(vec3(tangent[0], binormal[0], normal[0]),
						vec3(tangent[1], binormal[1], normal[1]),
						vec3(tangent[2], binormal[2], normal[2]));
		
		// Transform the light vector according to this matrix
		oTSLightDir = rotation * lightDir;

		// Calculate half-angle in tangent space
		eyeDir = normalize((vec4(eyePosition, 1.0) - (modelMatrix * fragPos))).xyz;
		
		// Get bump map vector, again expand from range-compressed
		bumpVec = expand(texture2D(normalMap, oUv0.xy).xyz);

		// Retrieve normalised light vector, expand from range-compressed
		lightVec = normalize(oTSLightDir).xyz;
		
		if (lightParams.x == 0.0)
		{
			spot = 1.0;
		}
		else
		{
			vec3 sdir = rotation * spotlightDir.xyz;
		
			spot = clamp((dot(oTSLightDir, normalize(-sdir)) - spotlightParams.y) / (spotlightParams.x - spotlightParams.y), 0.0, 1.0);
		}
		
		//Compute attenuation value
		att = 1.0;
		if (lightPosition.w > 0.0)
		{
			att = smoothstep(lightAttenuation.x, 0.0, length(lightPosition - (modelMatrix * fragPos))) * lightPower;
		}
		else
		{
			att = lightPower;
		}
		
		ndot = clamp(dot(bumpVec, lightVec), 0.0, 1.0);

		vec3 halfAngle = normalize(eyeDir + lightDir);
		oTSHalfAngle = rotation * halfAngle;
		
		// retrieve half angle and normalise
		halfAngle = normalize(oTSHalfAngle);
		
		// Pre-raise the specular exponent to the eight power
		specFactor = pow(clamp(dot(bumpVec, halfAngle), 0.0, 1.0), shininess);
	}

	//0: Ambient pass
	//1: Texture and lighting pass

	if (passNumber == 0)
		gl_FragColor = diffuse * vec4(ambient.xyz, diffuse.a);
	else if (passNumber == 1)
	{
		vec4 shadowUV = oUv;
		shadowUV = shadowUV / shadowUV.w;
		float centerdepth = texture2D(shadowMap, shadowUV.xy).x;
		
		float pixeloffset = inverseShadowmapSize;
		vec4 depths = vec4(
			texture2D(shadowMap, shadowUV.xy + vec2(-pixeloffset, 0)).x,
			texture2D(shadowMap, shadowUV.xy + vec2(+pixeloffset, 0)).x,
			texture2D(shadowMap, shadowUV.xy + vec2(0, -pixeloffset)).x,
			texture2D(shadowMap, shadowUV.xy + vec2(0, +pixeloffset)).x);
			
		vec2 differences = abs( depths.yw - depths.xz );
		float gradient = min(gradientClamp, max(differences.x, differences.y));
		float gradientFactor = gradient * gradientScaleBias;
		
		float depthAdjust = gradientFactor + (fixedDepthBias * centerdepth);
		float finalCenterDepth = centerdepth + depthAdjust;
		
		depths += depthAdjust;
		float final = (finalCenterDepth > shadowUV.z) ? 1.0 : 0.0;
		final += (depths.x > shadowUV.z) ? 1.0 : 0.0;
		final += (depths.y > shadowUV.z) ? 1.0 : 0.0;
		final += (depths.z > shadowUV.z) ? 1.0 : 0.0;
		final += (depths.w > shadowUV.z) ? 1.0 : 0.0;
		
		final *= 0.2;
		
		gl_FragColor = vec4((((diffuse * lightDiffuse * ndot) + (lightSpecular * specularColor * specFactor)) * att * spot).xyz * final, diffuse.a);
		//gl_FragColor = (finalCenterDepth > shadowUV.z) ? gl_FragColor : vec4(0, 0, 0, 1);
	}
}
