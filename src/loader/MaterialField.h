#pragma once

namespace loader {
    struct MaterialField {
        bool textureSpec : 1 { false };

        bool pattern : 1 { false };
        bool reflection : 1 { false };
        bool refraction : 1 { false };
        bool refractionRatio : 1 { false };

        bool tilingX : 1 { false };
        bool tilingY : 1 { false };

        bool spriteCount : 1 { false };
        bool spritesX : 1 { false };

        //bool ns : 1 { false };

        //bool ka : 1 { false };

        bool kd : 1 { false };
        bool ks : 1 { false };
        bool ke : 1 { false };

        bool map_bump_strength : 1 { false };

        //bool ni : 1 { false };
        //bool d : 1 { false };
        //bool illum : 1 { false };

        bool layers : 1 { false };
        bool layersDepth : 1 { false };
        bool parallaxDepth : 1 { false };

        bool metal : 1 { false };

        bool alpha : 1 {false};
        bool blend : 1 {false};

        bool renderBack : 1 {false};
        bool lineMode : 1 {false};
        bool reverseFrontFace : 1{false};

        bool gbuffer : 1 {false};
    };
}
