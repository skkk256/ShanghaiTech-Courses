#version 430 core

uniform vec3 CameraPos;
uniform bool DrawLine;

in VS_OUT {
    vec3 position;
    vec3 normal;
} fs_in;

out vec4 FragColor;

vec3 CalculateLightContributionPhong(
    vec3 normal,
    vec3 viewDirection,
    vec3 lightDirection,
    vec3 lightColor,
    vec3 diffuseReflectance,
    vec3 specularReflectance,
    float shininess
);

void main()
{
    if (DrawLine) {
        FragColor = vec4(0.5, 0.5, 0.5, 1);
    } else {
        vec3 viewDirection = normalize(fs_in.position - CameraPos);

        // Material settings
        vec3 diffuseReflectance = vec3(0.8, 0.1, 0.1);
        vec3 specularReflectance = vec3(0.8, 0.8, 0.8);
        float shininess = 16.0;

        // Ambient contribution
        vec3 color = vec3(0.2, 0.2, 0.2) * diffuseReflectance;

        // Light 1 contribution
        vec3 light1Direction = normalize(vec3(0.0, -0.6, -1.0)); // This is a directional light
        vec3 light1Color = vec3(1.0, 1.0, 1.0);
        color += CalculateLightContributionPhong(
            fs_in.normal,
            viewDirection,
            light1Direction,
            light1Color,
            diffuseReflectance,
            specularReflectance,
            shininess);

        // Light 2 contribution
        vec3 light2Direction = normalize(vec3(0.0, -0.6, 1.0)); // This is also a directional light
        vec3 light2Color = vec3(1.0, 1.0, 1.0);
        color += CalculateLightContributionPhong(
            fs_in.normal,
            viewDirection,
            light2Direction,
            light2Color,
            diffuseReflectance,
            specularReflectance,
            shininess);

        // You may add more lights

        FragColor = vec4(color, 1);
    }
}

vec3 CalculateLightContributionPhong(
    vec3 normal,
    vec3 viewDirection,
    vec3 lightDirection,
    vec3 lightColor,
    vec3 diffuseReflectance,
    vec3 specularReflectance,
    float shininess
) {
    // Double-sided shading, determine which side we're looking at
    if (dot(-viewDirection, normal) < 0) {
        normal = -normal;
    }

    float dpLightNormal = dot(-lightDirection, normal);
    if (dpLightNormal <= 0.0) {
        return vec3(0.0);
    }

    vec3 diffuse = dpLightNormal * diffuseReflectance * lightColor;

    vec3 specular;
    const bool useBlinn = false;
    if (useBlinn) {
        vec3 halfwayDirection = -normalize(viewDirection + lightDirection);
        float dpHalfwayNormal = dot(halfwayDirection, normal);

        specular = pow(max(dpHalfwayNormal, 0.0), shininess) * specularReflectance * lightColor;
    } else {
        vec3 reflectDirection = reflect(-lightDirection, normal);

        float dpReflectView = dot(reflectDirection, -viewDirection);
        specular = pow(max(dpReflectView, 0.0), shininess) * specularReflectance * lightColor;
    }

    return diffuse + specular;
}