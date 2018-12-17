#version 330 core
in vec2 texCoord;

uniform sampler2D texture;
uniform vec2 blurRadius;

const int Quality = 8;
const int Directions = 8;
const float PIx2 = 6.28318530718;

void main(void) {
    vec4 Color = texture2D(texture, texCoord);
    for(float d = 0.0; d < PIx2; d += PIx2/float(Directions)) {
        for(float i= 1.0/float(Quality); i <= 1.0; i += 1.0/float(Quality)) {
            Color += texture2D(texture, texCoord + vec2(cos(d), sin(d))*blurRadius*i);
        }
    }
    Color /= float(Quality)*float(Directions) + 1.0;
    gl_FragColor =  Color;
}