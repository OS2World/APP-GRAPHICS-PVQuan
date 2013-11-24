#include "colors.inc"
#include "textures.inc"
#include "shapes.inc"

object {             /* bounce surface */
   plane { <0 1 0> @floor_height@ }
   texture {
		checker colour White colour Black
		phong 0.8
		phong_size 80
		reflection 0.5
	}
}

object {         /* ball */
	quadric {
		<@x_size@ @y_size@ @z_size@>
		<0 0 0>
		<0 0 0>
		-1
	}
	texture { colour Red }
	translate <@ball_x@ @height@ @ball_z@>
}

object {     /* light source */
   light_source { 
      <@light_x@ @light_y@ @light_z@>
      colour White 
   }
}

camera {
   location <0 0 0>
   direction <0 0 1>
   up <0 1 0>
   right <1.3333 0 0>
}
