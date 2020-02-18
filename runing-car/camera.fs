#version 330 core
out vec4 FragColor;

//in vec2 TexCoord;
in vec3 Normal;  
in vec3 FragPos;

// texture samplers
//uniform sampler2D texture1;
//uniform sampler2D texture2;

uniform vec3 lightPos; 
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

void main()
{

	//FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);
    // ambient
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse cosA
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // specular cosA^32
    float specularStrength = 1.1;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    
    vec3 result = (ambient + diffuse + specular) * objectColor;
    //vec3 result = ambient * objectColor;
    FragColor = vec4(result, 0.5);

    //FragColor = vec4(0.92f, 0.89f, 0.41f, 1.0f);
}