out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;


uniform vec3 viewPos;


vec3 phong_shade(vec3 light_direction, vec3 look, vec3 normal, vec3 ka, vec3 kd, vec3 ks, float shininess) {
    vec3 R = reflect(-light_direction, normal);

    float diffuse = max(0.0, dot(light_direction, normal));
    float specular = pow(max(0.0, dot(R, look)), shininess);

    return ka + kd * diffuse + ks * specular;
}

vec3 shade(vec3 pos, vec3 look, vec3 normal, vec3 ka, vec3 kd, vec3 ks, float shininess) {
    return ka +
    phong_shade(normalize(vec3(-0.5, 0.8, 0.0) - pos), look, normal, vec3(0.0), kd, ks, shininess) +
    phong_shade(normalize(vec3( 0.5, 0.8, 0.0) - pos), look, normal, vec3(0.0), kd, ks, shininess);
}

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 ka = vec3(0.9, 0.9, 0.9);
    vec3 kd = vec3(0.6, 0.6, 0.6);
    vec3 ks = vec3(1.0, 1.0, 1.0);
    float shininess = 32.0;

    vec3 result = shade(FragPos, viewDir, norm, ka, kd, ks, shininess);

    FragColor = vec4(result, 1.0);
}