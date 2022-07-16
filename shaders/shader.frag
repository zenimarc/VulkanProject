#version 450

layout(set = 1, binding = 1) uniform sampler2D texSampler;

layout(set=0, binding = 0) uniform globalUniformBufferObject {
    mat4 view;
    mat4 proj;
    float time;
    vec3 eyePos;
    vec3 cameraDir;
    vec4 coneInOutDecayExp;
} gubo;

layout(set=1, binding = 0) uniform UniformBufferObject {
    mat4 model;
    int isFlowingColor;
    vec3 highlightColor;
} ubo;

layout(location = 0) in vec3 fragViewDir;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragTexCoord;
layout(location=3) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

// we need to compute the direction from the light position (gubo.lightPosition) and the object position (pos)
vec3 point_light_dir(vec3 pos, vec3 lightPos) {
    // Point light direction
    return (lightPos-pos)/length(lightPos-pos);
}

// we have to compute the color and intensity received at the pos of the object using the formulas on the slides
vec3 point_light_color(vec3 pos, vec3 lightPos) {
    // Point light color
    vec3 lightColor = vec3(0.964f, 0.603f, 0.329f);
    float g = 12.0f;
    float beta = 1.0f;
    return lightColor * pow((g/(length(lightPos-pos))), beta);
}


void main() {
    
    const vec3  diffColor = texture(texSampler, fragTexCoord).rgb;
    const vec3  ambientColor = vec3(0.1f, 0.1f, 0.1f);
    const vec3  specColor = vec3(1.0f, 1.0f, 1.0f);
    const float specPower = 150.0f;
    
    vec3 N = normalize(fragNorm); // Normal vector
    vec3 V = normalize(fragViewDir);  // View direction
    
    // ambient
    vec3 ambient  = ambientColor * diffColor;
    
    //point light (torch of the character)
    //vec3 p1_pos = vec3(4.20f, 12.70f, 0.37f);
    vec3 p1_pos = gubo.eyePos + vec3(0.0f, 0.5f, 0.0f);
    vec3 p1_lD = point_light_dir(fragPos, p1_pos);
    vec3 p1_color = point_light_color(fragPos, p1_pos);
    vec3 p1_specular = specColor * pow(max(dot(-reflect(p1_lD, N),V), 0.0f), specPower);
    vec3 p1_diffuse = diffColor * max(dot(N,p1_lD), 0.0f);
    vec3 p1_final = p1_color * (p1_diffuse);

    
    outColor = vec4(clamp(0.2f*ambient + 1.2*p1_final + 0.5*ubo.highlightColor, vec3(0.0f), vec3(1.0f)), 1.0f);
}
