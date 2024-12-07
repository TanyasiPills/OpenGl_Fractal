#shader vertex
#version 330 core

layout(location = 0) in vec4 position;

out vec2 fragTexCoord;

void main()
{
	gl_Position = position;
	fragTexCoord = (position.xy + 1.0) * 0.5;
}

#shader fragment
#version 330 core

uniform int maxIterations; 
uniform int set;
uniform float zoom; 
uniform float tScale;
uniform vec2 offset;  
uniform vec3 colorYey;

in vec2 fragTexCoord;

layout(location = 0) out vec4 color;

void main()
{
    vec2 c = (fragTexCoord - vec2(0.5)) * 2.0 / zoom + offset;

    vec2 z = vec2(0.0);
    int iterations = 0;
    if(set == 0)
        while (length(z) <= 2.0 && iterations < maxIterations)
        {
            float x = (z.x * z.x - z.y * z.y) + c.x;
            float y = (2.0 * z.x * z.y) + c.y;
            z = vec2(x, y);
            iterations++;
        }
    else if(set == 1){

        while (length(z) <= 2.0 && iterations < maxIterations)
        {
            float x = (abs(z.x) * abs(z.x) - abs(z.y) * abs(z.y)) + c.x;
            float y = (2.0 * abs(z.x) * abs(z.y)) + c.y;
            z = vec2(x, y);
            iterations++;
        }
    }
    else if (set == 2){
        c = vec2(-0.70176, -0.3842) / zoom + offset;
        z = fragTexCoord * 2.0 - vec2(1.0, 1.0);

        while (length(z) <= 2.0 && iterations < maxIterations)
        {
            float x = (z.x * z.x - z.y * z.y) + c.x;
            float y = (2.0 * z.x * z.y) + c.y;
            z = vec2(x, y);
            iterations++;
        }
    }

    if (iterations == maxIterations)
    {
        color = vec4(0.0, 0.0, 0.0, 1.0); // Black
    }
    else
    {
        float t = float(iterations) / float(maxIterations) * tScale;
        color = vec4(t*colorYey.x, t*colorYey.y, t*colorYey.z, 1.0);
    }
}