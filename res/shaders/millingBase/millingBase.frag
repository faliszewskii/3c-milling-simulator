#version 400 core
out vec4 fragColor;

in vec3 normal;
in vec3 fragPos;
in vec2 texCoords;
in float color;

uniform vec3 viewPos;
uniform bool useColorMap;

struct Material {
    bool hasTexture;
    sampler2D texture;
    float shininess;
    vec4 albedo;
};
uniform Material material;

struct PointLight {
    vec3 position;
    vec3 color;
    float strength;
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
};
uniform PointLight pointLight;

vec3 calculateLight(PointLight light, vec3 N, vec3 fragPos, vec3 viewDir) {

    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(N, lightDir), 0.0);
    // specular shading
    vec3 halfwayDir = max(normalize(lightDir + viewDir), 0);

    // attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / ((distance * distance)* light.quadraticAttenuation + distance*light.linearAttenuation + light.constantAttenuation);

    float spec = material.shininess != 0 ? pow(max(dot(N, halfwayDir), 0.0), material.shininess): 0;
    // combine results
    vec3 ambient  = light.color * vec3(0.2f);
    vec3 diffuse  = light.color * diff;
    vec3 specular = light.color * spec;
    return ambient + (diffuse + specular) * light.strength ;//* attenuation;
}

void main() {
    vec4 objectColor;
    if(useColorMap) {
        if(color >= -0.05 && color <= 0.05) {
            objectColor = vec4(vec3(0, (0.05-abs(color))/0.05, 0), 1);
        }
        else if(color < 0) {
            objectColor = vec4(vec3(-color, 0.05, 0), 1);
        } else {
            objectColor = vec4(vec3(0, 0.05, color), 1);
        }
    } else {
        objectColor = material.hasTexture? texture(material.texture, texCoords) : material.albedo;
    }

    vec3 normNormal = normalize(normal);

    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 result = vec3(0);

    result += calculateLight(pointLight, normNormal, fragPos, viewDir);
    result *= objectColor.rgb;

    result =  pow(result, vec3(1.0/2.2f));
    fragColor = vec4(result, objectColor.a);
}