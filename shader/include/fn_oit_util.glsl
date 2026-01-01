#ifndef OIT_DISCARD
#define OIT_DISCARD(alpha) if (alpha < u_oitMinBlendThreshold || alpha >= u_oitMaxBlendThreshold) discard;
#endif
