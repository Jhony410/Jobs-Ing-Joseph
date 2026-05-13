#version 330 core
out vec4 FragColor;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 

in vec3 Normal;  
in vec3 FragPos;

uniform vec3 lightPos;    
uniform vec3 viewPos; 
uniform vec3 lightColor;  
uniform Material material; 

void main() {
    // AMBIENTAL
    vec3 ambient = lightColor * material.ambient;
  	
    // DIFUSA
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos); 
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = lightColor * (diff * material.diffuse);
    
    // ESPECULAR
    vec3 viewDir = normalize(viewPos - FragPos); 
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess * 128.0); 
    vec3 specular = lightColor * (spec * material.specular);  
        
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}