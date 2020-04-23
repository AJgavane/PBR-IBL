#version 450 core

out vec4 FragColor;
in vec3 v_worldPos;

uniform samplerCube environmentMap;

void main()
{		
    vec3 envColor = texture(environmentMap, v_worldPos).rgb;
    
    // HDR tonemap and gamma correct
    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0/2.2)); 
    
     FragColor = vec4(envColor, 1.0);
    // FragColor = vec4(0.9, 0.2, 0.2, 1.0);
    //FragColor = vec4(texture(environmentMap, v_worldPos).rg, 0.0, 1.0);
}