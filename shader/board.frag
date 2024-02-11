#version 460 core

in vec3 fragVertexColor;
in vec2 fragTexCoord;
in vec2 fragBoardCoord;
out vec4 FragColor;
uniform sampler2DArray figures;
uniform uint position[64];

void main()
{   
    vec4 finalColor;
    uint figure = position[uint(fragBoardCoord.y*8+fragBoardCoord.x)];
    bool selected = (figure & (1<<8)) !=0;
    bool available_move = (figure & (1<<9)) !=0;
    figure &= 0xFF;

    if (figure==0) {
            finalColor = vec4(fragVertexColor, 1.0);
    } else {
        // Get texture color and alpha at the specified index
        vec4 texColor = texture(figures, vec3(fragTexCoord, figure-1));
        float alpha = texColor.a;
       
       // Mix base and texture colors based on alpha
        finalColor = vec4(mix(fragVertexColor, texColor.rgb, alpha), 1.0);                
    }
    if (selected || available_move) {
        float borderThickness = 0.03;
        vec4 borderColor;
        if (selected) {         // draw rect
            
            borderColor=vec4(0.1, 0.5, 1.0, 1.0);
            vec2 bl = step(vec2(borderThickness), fragTexCoord);        // bottom left
            vec2 tr = step(vec2(borderThickness), 1.0-fragTexCoord);    // top right
            float border = bl.x * bl.y * tr.x * tr.y;
        
            finalColor = mix(borderColor, finalColor, border);
        }    
        else if (available_move){   // draw circle
            
            borderColor=vec4(1.0, 0.0, 0.05, 0.0);

            vec2 dist = fragTexCoord-vec2(0.5);
            float r = 1.0;   
            float circle = 1.0-smoothstep(r-(r*0.01), r+(r*0.01), dot(dist, dist)*4.0);
            vec4 circleColor = mix(finalColor, borderColor, circle);
            r = 0.9;
            circle = 1.0-smoothstep(r-(r*0.01), r+(r*0.01), dot(dist, dist)*4.0);
            finalColor = mix(circleColor, finalColor, circle); 
        } 
    }
    FragColor = finalColor;
}