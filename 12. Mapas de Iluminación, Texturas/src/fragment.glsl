#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;  
    sampler2D specular; 
    float shininess;
}; 

in vec3 Normal;  
in vec3 FragPos;
in vec2 TexCoords; 

uniform vec3 lightPos;    
uniform vec3 viewPos; 
uniform vec3 lightColor;  
uniform Material material; 

void main() {
    vec3 colorBase = vec3(texture(material.diffuse, TexCoords));
    vec3 ambient = lightColor * 0.2 * colorBase; // 20% de fuerza
  	
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos); 
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = lightColor * diff * colorBase;
    
    vec3 viewDir = normalize(viewPos - FragPos); 
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess * 128.0);
    
    vec3 mapaBlancoNegro = vec3(texture(material.specular, TexCoords));
    vec3 specular = lightColor * spec * mapaBlancoNegro;  
        
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}