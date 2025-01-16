#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 viewPos;
uniform vec3 objectColor;

uniform vec3 lightPos;
uniform vec3 lightColor;

uniform vec3 lightPos2;
uniform vec3 lightColor2;

void main()
{
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    // second light
    float ambientStrength2 = 0.1;
    vec3 ambient2 = ambientStrength2 * lightColor;
    vec3 lightDir2 = normalize(lightPos2 - FragPos);
    float diff2 = max(dot(norm, lightDir2), 0.0);
    vec3 diffuse2 = diff2 * lightColor2;
    float specularStrength2 = 1;
    vec3 reflectDir2 = reflect(-lightDir2, norm);
    float spec2 = pow(max(dot(viewDir, reflectDir2), 0.0), 16);
    vec3 specular2 = specularStrength2 * spec2 * lightColor2;

    vec3 result = (ambient + diffuse + specular + ambient2 + diffuse2 + specular2) * objectColor;
    FragColor = vec4(result, 1.0);
}
