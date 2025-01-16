#version 330 core
out vec4 FragColor;
uniform vec2 mousePos;

void main()
{
    float dist = distance(gl_FragCoord.xy, mousePos);
    if (dist < 10.0) // 50.0是圆的半径
        FragColor = vec4(0.7, 0.7, 0.0, 1.0);
    else
        discard;
}