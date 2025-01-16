// Fragment Shader
#version 330 core
out vec4 FragColor;

uniform vec2 mousePos;

void main()
{
    float dist = distance(gl_FragCoord.xy, mousePos);
    vec4 ObjectColor;

    if (dist < 10) // 50.0是圆的半径
        ObjectColor = vec4(1.0, 0.0, 0.0, 1.0); // 红色
    else
        ObjectColor = vec4(1.0, 1.0, 1.0, 1.0); // 球体颜色

    FragColor = ObjectColor; // 球体颜色
}