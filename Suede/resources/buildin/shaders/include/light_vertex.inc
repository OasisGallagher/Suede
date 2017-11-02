out vec4 shadowCoord;
uniform mat4 c_localToShadowSpaceMatrix;

#define calculateShadowCoord()	shadowCoord = c_localToShadowSpaceMatrix * vec4(c_position, 1)
