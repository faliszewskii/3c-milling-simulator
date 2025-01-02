#version 400 core
out vec4 fragColor;

in vec3 normal;
in vec3 fragPos;
in vec2 texCoords;
in float color;

uniform vec3 viewPos;
uniform bool useColorMap;
uniform float errorMargin;

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
        float r = 0;
        float g = 0;
        float b = 0;
        float error = errorMargin / 35;
        if(color >= -error && color <= error) {
            g =  0.5 + (error-abs(color))/error;
            if(g > 1) g = 1;
        }
        else if(color < 0) {
            r = 0.5 + -color;
            if(r > 1) r = 1;
        } else {
            b = 0.5 + color;
            if(b > 1) b = 1;
        }
        objectColor = vec4(vec3(r, g, b) + vec3(0.2), 1);
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