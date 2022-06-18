#version 450

layout(set = 1, binding = 1) uniform sampler2D texSampler;

layout(set=0, binding = 0) uniform globalUniformBufferObject {
    mat4 view;
    mat4 proj;
    float time;
    vec3 eyePos;
} gubo;

layout(set=1, binding = 0) uniform UniformBufferObject {
    mat4 model;
    int isFlowingColor;
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
    float g = 5.0f;
    float beta = 1.0f;
    return lightColor * pow((g/(length(lightPos-pos))), beta);
}

void main() {
    vec3 a = vec3(0.5, 0.5, 0.5);
    vec3 b = vec3(0.5, 0.5, 0.5);
    vec3 c = vec3(1, 1, 1);
    vec3 d = vec3(0.00, 0.33, 0.67);
    vec3 flowingColor = ubo.isFlowingColor * (a + b * cos( 2*3.14*(c*gubo.time+d)));
    
    const vec3  diffColor = texture(texSampler, fragTexCoord).rgb;
    const vec3  ambientColor = vec3(0.1f, 0.1f, 0.1f);
	const vec3  specColor = vec3(0.964f, 0.603f, 0.329f);
	const float specPower = 150.0f;
	const vec3  L = vec3(-0.4830f, 0.8365f, -0.2588f); // light direction
	
	vec3 N = normalize(fragNorm); // Normal vector
	vec3 R = -reflect(L, N); // Reflected vector (used for specular)
	vec3 V = normalize(fragViewDir);  // View direction
	
    //point light 1
    vec3 p1_pos = vec3(4.20f, 12.70f, 0.37f);
    vec3 lD = point_light_dir(fragPos, p1_pos);
    vec3 p1_color = point_light_color(fragPos, p1_pos);
    vec3 p1_specular = specColor * pow(max(dot(-reflect(lD, N),V), 0.0f), specPower);
    vec3 p1_diffuse = p1_color * diffColor * max(dot(N,lD), 0.0f);
    
	// Lambert diffuse
	vec3 diffuse  = diffColor * max(dot(N,L), 0.0f);
	// Phong specular
	vec3 specular = specColor * pow(max(dot(R,V), 0.0f), specPower);
	// ambient
	vec3 ambient  = ambientColor * diffColor;
	
	outColor = vec4(clamp(ambient + p1_diffuse + p1_specular + 0.7*flowingColor, vec3(0.0f), vec3(1.0f)), 1.0f);
}
