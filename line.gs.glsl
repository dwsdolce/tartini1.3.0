#version 330
layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;
in vec4 geom_color[];
    
uniform float width;
uniform vec2 screen_size;

out vec4 frag_color;

void main(){
   float alfa, deltax, deltay, LWX, LWY, x_offset, y_offset;

   // Calculate the pixel width of the line in terms of the range -1 to 1 based on the screen size
   float screen_width = screen_size.x;
   float screen_height = screen_size.y;
   LWX = width / screen_width;
   LWY = width / screen_height;

   // Caclulate the x and y delts of the line
   deltax = gl_in[1].gl_Position.x - gl_in[0].gl_Position.x;
   deltay = gl_in[1].gl_Position.y - gl_in[0].gl_Position.y;
   // Calculate the angle of the line in radians.
   if (deltax == 0) {
      alfa = 0; // Vertical line
   } else {
      // pi/2 - angle of the line from x = angle from the y axis
      alfa = 1.5707963 - atan(deltay, deltax);
   }
   x_offset = LWX * cos(alfa);
   y_offset = LWY * sin(alfa);
   gl_Position = vec4(gl_in[0].gl_Position.x - x_offset, gl_in[0].gl_Position.y + y_offset, 0.0, 1.0);
   frag_color = geom_color[0];
   EmitVertex();
   gl_Position = vec4(gl_in[0].gl_Position.x + x_offset, gl_in[0].gl_Position.y - y_offset, 0.0, 1.0);
   frag_color = geom_color[0];
   EmitVertex();
   gl_Position = vec4(gl_in[1].gl_Position.x - x_offset, gl_in[1].gl_Position.y + y_offset, 0.0, 1.0);
   frag_color = geom_color[0];
   EmitVertex();
   gl_Position = vec4(gl_in[1].gl_Position.x + x_offset, gl_in[1].gl_Position.y - y_offset, 0.0, 1.0);
   frag_color = geom_color[0];
   EmitVertex();
   EndPrimitive();
}
