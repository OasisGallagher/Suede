out vec4 shadowCoord;
uniform mat4 c_localToShadowMatrix;

#define calculateShadowCoord()	shadowCoord = c_localToShadowMatrix * vec4(c_position, 1)
