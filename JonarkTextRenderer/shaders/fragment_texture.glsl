#version 450 core

struct Material {
    vec3 ambient_color;
    vec3 diffuse_color;
    vec3 specular_color;
    sampler2D texture_diffuse_1;
    float shininess;
};

struct PointLight {
    vec3 color;
    vec3 position;
};

const int MAX_NUM_POINT_LIGHTS = 100;

layout (location = 0) out vec4 fColor;

uniform Material material;
uniform PointLight point_lights[MAX_NUM_POINT_LIGHTS];
uniform int num_point_lights;
uniform vec3 viewPos;

in vec2 texPos;
in vec3 fragPos;
in vec3 normal;

vec3 ComputePointLight(PointLight point_light, vec3 view_dir) {
    vec3 light_dir = normalize(point_light.position - fragPos);
    float nDotL = dot(normal, light_dir);
    vec3 diffuse = material.diffuse_color * max(nDotL, 0.0)
    * vec3(texture(material.texture_diffuse_1, texPos)); // TODO: Use the rest of diffuse maps

    vec3 h = normalize(light_dir + view_dir);
    float nDotH = dot(normal, h);
    vec3 specular = point_light.color * material.specular_color * pow(max(nDotH, 0.0), material.shininess); // TODO: Use specular map

    return diffuse + specular;
}

void main() {
    vec3 view_dir = normalize(viewPos - fragPos);

    vec3 illumination = vec3(0.0);
    vec4 texColor = texture(material.texture_diffuse_1, texPos);
    vec3 ambient = material.ambient_color * texColor.rgb;
    illumination += ambient;
    for (int i = 0; i < min(num_point_lights, MAX_NUM_POINT_LIGHTS); i++) {
        illumination += ComputePointLight(point_lights[i], view_dir);
    }

    fColor = vec4(illumination, texColor.a);
}