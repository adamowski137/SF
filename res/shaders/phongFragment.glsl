#version 430 core

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 cameraPos;
uniform vec4 color;

void main()
{
	vec3 norm = normalize(Normal);

	float ambientStrength = 0.1;
	vec3 ambient = vec3(ambientStrength, ambientStrength, ambientStrength);

	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = vec3(diff, diff, diff);

	float specularStrength = 0.5;
	vec3 viewDir = normalize(cameraPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = vec3(specularStrength * spec, specularStrength * spec, specularStrength * spec);

	FragColor = vec4(ambient + diffuse + specular, 1.0) * color;
}
