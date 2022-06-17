#version 450

layout(set = 1, binding = 1) uniform sampler2D texSampler;

layout(set=0, binding = 0) uniform globalUniformBufferObject {
    mat4 view;
    mat4 proj;
    float time;
} gubo;

layout(set=1, binding = 0) uniform UniformBufferObject {
    mat4 model;
    int isFlowingColor;
} ubo;

layout(location = 0) in vec3 fragViewDir;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 a = vec3(0.5, 0.5, 0.5);
    vec3 b = vec3(0.5, 0.5, 0.5);
    vec3 c = vec3(1, 1, 1);
    vec3 d = vec3(0.00, 0.33, 0.67);
    vec3 flowingColor = ubo.isFlowingColor * (a + b * cos( 2*3.14*(c*gubo.time+d)));
    
    const vec3  diffColor = texture(texSampler, fragTexCoord).rgb;
    const vec3  ambientColor = vec3(0.3f, 0.3f, 0.3f);
	const vec3  specColor = vec3(1.0f, 1.0f, 1.0f);
	const float specPower = 150.0f;
	const vec3  L = vec3(-0.4830f, 0.8365f, -0.2588f); // light direction
	
	vec3 N = normalize(fragNorm); // Normal vector
	vec3 R = -reflect(L, N); // Reflected vector (used for specular)
	vec3 V = normalize(fragViewDir);  // View direction
	
	// Lambert diffuse
	vec3 diffuse  = diffColor * max(dot(N,L), 0.0f);
	// Phong specular
	vec3 specular = specColor * pow(max(dot(R,V), 0.0f), specPower);
	// ambient
	vec3 ambient  = ambientColor * diffColor;
	
	outColor = vec4(clamp(ambient + diffuse + 0.7*flowingColor + specular, vec3(0.0f), vec3(1.0f)), 1.0f);
}
