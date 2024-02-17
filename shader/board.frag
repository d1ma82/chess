#version 460 core

in vec3 fragVertexColor;
in vec2 fragTexCoord;
in vec2 fragBoardCoord;
out vec4 FragColor;
uniform sampler2DArray figures;
uniform uint position[64];

void main()
{   
    vec4 finalColor = vec4(fragVertexColor, 1.0);
    uint figure = position[uint(fragBoardCoord.y*8+fragBoardCoord.x)];
    bool selected = (figure & uint(1<<8)) !=0;
    bool available_move = (figure & uint(1<<9)) !=0;
    bool check = (figure & uint(1<<11)) !=0;
    figure &= 0xFF;

    //vec3 color = move? vec3(0.88, 0.72, 0.5): fragVertexColor;
    
    vec3 color;
    
    if (check) {
        color=vec3(1.0, 0.0, 0.0);
        float pct = distance(fragTexCoord, vec2(0.5));
        finalColor = vec4(mix(fragVertexColor, color, pct), 1.0);
    }
    else if (selected) {         // draw rect
        
        float thickness = 0.05;
        color=vec3(0.1, 0.5, 1.0);
        vec2 bl = step(vec2(thickness), fragTexCoord);        // bottom left
        vec2 tr = step(vec2(thickness), 1.0-fragTexCoord);    // top right
        float border = bl.x * bl.y * tr.x * tr.y;
    
        finalColor = vec4(mix(color, fragVertexColor, border), 1.0);
    }    
    if (available_move){   // draw circle
        
        color=vec3(1.0, 0.0, 0.05);
        vec2 dist = fragTexCoord-vec2(0.5);
        float r = 1.0;   
        float circle = 1.0-smoothstep(r-(r*0.01), r+(r*0.01), dot(dist, dist)*4.0);
        vec3 circleColor = mix(finalColor.rgb, color, circle);
        r = 0.85;
        circle = 1.0-smoothstep(r-(r*0.01), r+(r*0.01), dot(dist, dist)*4.0);
        finalColor = vec4(mix(circleColor, finalColor.rgb, circle), 1.0); 
    } 
    if (figure!=0) {
        // Get texture color and alpha at the specified index
        vec4 texColor = texture(figures, vec3(fragTexCoord, figure-1));
        float alpha = texColor.a;
       
       // Mix base and texture colors based on alpha
        finalColor = vec4(mix(finalColor.rgb, texColor.rgb, alpha), 1.0);                
    }
    FragColor = finalColor;
}