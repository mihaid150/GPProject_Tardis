#version 410 core


in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;

//lighting
uniform vec3 lightColor;
uniform vec3 sunPosition;

// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

// Attenuation coefficients
const float constant = 1.0;
const float linear = 0.09;
const float quadratic = 0.032;

//components
vec3 ambient;
float ambientStrength = 0.1f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.1f;
float shadow;

float computeShadow() 
{
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    normalizedCoords = normalizedCoords * 0.5 + 0.5;
    
    if (normalizedCoords.z > 1.0f)
        return 0.0f;
    
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
    float currentDepth = normalizedCoords.z;
    
    float bias = max(0.05f * (1.0f - dot(normalize(fNormal), sunPosition)), 0.005f);
    float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;

    return shadow;
}

void computeDirLight()
{
    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //normalize light direction
     vec3 lightDirN = normalize(vec3(view * vec4(sunPosition, 1.0f)) - fPosEye.xyz);

    //compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDir = normalize(- fPosEye.xyz);

    // Compute distance to light
    float distance = length(sunPosition - fPosition);
    float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));

    // add shadow calculation
    shadow = computeShadow();

    //compute ambient light
    ambient = ambientStrength * lightColor;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor * attenuation;
    diffuse *= (1.0f - shadow) * texture(diffuseTexture, fTexCoords).rgb;

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular = specularStrength * specCoeff * lightColor * attenuation;
    specular *= (1.0f - shadow) * texture(specularTexture, fTexCoords).rgb;
}

void main() 
{
    computeDirLight();

    //compute final vertex color
    vec3 color = min((ambient + diffuse) * texture(diffuseTexture, fTexCoords).rgb + specular * texture(specularTexture, fTexCoords).rgb, 1.0f);

    fColor = vec4(color, 1.0f);
}