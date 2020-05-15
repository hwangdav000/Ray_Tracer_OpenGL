#version 330 core

layout (location=0) out vec4 out_fragcolor;

in vec3 position;
in vec3 color;
in vec3 normal;

void main(){
    
    // Color coeffs
    float ka = 0.3f;
    float kd = 0.6f;
    
    // define static point light source at (0,0,0)
    vec3 L = -1.f*normalize(vec3(position));
    
    // define directional light source
    // vec3 L = normalize(vec3(1,1,1));

    // compute Phong illumination (diffuse only)
	vec3 N = normalize(normal);
    
    // draw two-sided
    float N_dot_L = dot(N, L);
	if ((N_dot_L) < 0.0) { N_dot_L *= -1.0; }
    
    // draw one-sided
    //float N_dot_L = max(0, dot(N, L));
    
    vec3 result = ka * color + kd * color * N_dot_L;
	out_fragcolor = vec4( result, 1.0 );
} 


