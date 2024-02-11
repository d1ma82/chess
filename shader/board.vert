#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in vec2 boardCoord;
out vec3 fragVertexColor;
out vec2 fragTexCoord;
out vec2 fragBoardCoord;

void main()
{   
    gl_Position = vec4(aPos, 1.0);
    fragVertexColor = color;
    fragTexCoord = texCoord;
    fragBoardCoord = boardCoord;
}