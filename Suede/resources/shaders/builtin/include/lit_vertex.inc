#ifndef __LIT_VERTEX_INC__
#define __LIT_VERTEX_INC__

out vec4 shadowCoord;
#define calculateShadowCoord()	shadowCoord = c_worldToShadowMatrix * c_localToWorldMatrix * vec4(c_position, 1)

#endif // __LIT_VERTEX_INC__
