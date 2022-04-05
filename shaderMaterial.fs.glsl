#version 410 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos;

uniform struct LightInfo {
  vec4 position; // Light position in eye coords.
  vec3 ambient;       // Ambient light intensity
  vec3 diffuse;       // Diffuse light intensity
  vec3 specular;       // Specular light intensity
} light;

uniform struct MaterialInfo {
  vec3 ambient;            // Ambient reflectivity
  vec3 diffuse;            // Diffuse reflectivity
  vec3 specular;            // Specular reflectivity
  float shininess;    // Specular shininess factor
} material;

void main() {
     // ambient
    vec3 ambient = light.ambient * material.ambient;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir;
    if ( light.position.w == 0.0 ) {
        lightDir = normalize(-light.position.xyz);
    } else {
        lightDir = normalize(light.position.xyz - FragPos);
    }
    float diff;
    if ( gl_FrontFacing ) {
        diff = max(dot(norm, lightDir), 0.0);
    } else {
        diff = max(dot(norm, -lightDir), 0.0);
    }
    vec3 diffuse = light.diffuse * (diff * material.diffuse);
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir;
    if ( gl_FrontFacing ) {
        reflectDir = reflect(-lightDir, norm); 
    } else {
        reflectDir = reflect(lightDir, norm); 
    }
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);  
        
    vec3 result = ambient + diffuse + specular;

    FragColor = vec4(result, 1.0);
}