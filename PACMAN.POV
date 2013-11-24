// Persistence of Vision Raytracer
// pacman.dat
// Based on pacman.dat by Ville Saari
// Copyright (c) 1991 Ferry Island Pixelboys
/*
* PvRay scene description for 'Pac Man doing his favourite job'
* 
* Created: 03-Jan-91
* Updated: 05-Jan-91
*
* You will need pacman.iff which is included in this package.
*
* If you do some nice modifications or additions to this file, please send 
* me a copy. My Internet address is:
*
*         vsaari@niksula.hut.fi
*/

#include "shapes.inc"
#include "colors.inc"

/* For 3d views, the following was used to calculate the right eye position,
  to give 5 degrees convergence at (40,10,0)

For left eye:
  Distance to focal point      = sqrt((40+80)^2+(0+140)^2)
                               = 184.39
  Direction focal point to eye = atan((40+80)/(0+140))
                               = -130.6 degrees

For right eye:
  Direction focal point to eye = -130.6 + 5
                               = -125.6 degrees
  x coordinate                 = 40 + cos(-125.6) * 184.39
                               = -67.34
  z coordinate                 = 0 + sin(-125.6) * 184.39
                               = -149.93
*/

#declare eye_pos            = <-80.0 35.0 -140>
// #declare eye_pos         = <-67.34 35.0 -149.93>   Uncomment for right eye

camera {			     
   location eye_pos

   direction <0.0 0.0 1.0>
   up  <0.0  1.0  0.0>
   right <1.1922 0.0 0.0>

   sky <0.0 1.0 0.0>
   look_at <40.0 10.0 0>
 }

object { /* The maze-textured ground */
   plane { <0.0 1.0 0.0> 0.0  }
   texture {
      image_map { <1.0 0.0 -1.0> iff "pacman.iff" 
       interpolate 2.0
       }
      scale <1600.0 1600.0 1600.0>
      translate <-196.0 0.0 160.0>
      ambient 0.3
      diffuse 0.7
      reflection 0.5
    }
 }

#declare eye = composite {
   object {
      sphere { <0.0 0.0 0.0> 6.0  }
      texture {
         colour red 1.0 green 1.0 blue 1.0
         ambient 0.3
         diffuse 0.7
         reflection 0.5
       }
    }

   object { /* Iris */
      sphere { <0.0 0.0 -2.3> 4.0  }
      texture {
         colour red 0.3 green 0.4 blue 0.8
         ambient 0.3
         diffuse 0.7
         reflection 0.5
       }
    }

   object { /* Pupil */
      sphere { <0.0 0.0 -5.5> 1.0  }
      texture {
         colour red 0.0 green 0.0 blue 0.0
         ambient 0.3
         diffuse 0.7
         reflection 0.5
       }
    }
 }

#declare food = object {
   sphere { <0.0 0.0 0.0> 4.0  }

   texture {
      colour red 1.0 green 1.0 blue 1.0
      ambient 0.3
      diffuse 0.7
      specular 2.0
      roughness 0.01
      reflection 0.6
    }
 }

composite { /* And now the world famous... Ta Da! ... pac man */

   object { /* Body */
      intersection {
         sphere { <0.0 0.0 0.0>  30.0  }
         sphere { <0.0 0.0 0.0>  28.0 inverse  }
         union {
            plane { <0.0 1.0 0.0> 0.0 rotate <@lip_lower@ 0.0 0.0>  }
            plane { <0.0 -1.0 0.0> 0.0 rotate <@lip_upper@ 0.0 0.0>  }
          }

         sphere { <0.0 0.0 -29.0> 1.5     /* Nostril */
            inverse
            rotate <16.0 -4.0 0.0>
          }

         sphere { <0.0 0.0 -29.0> 1.5     /* Nostril */
            inverse
            rotate <16.0 4.0 0.0>
          }
       }

      texture {
         colour red 0.9 green 0.8
         ambient 0.3
         diffuse 0.7
         specular 0.5
         roughness 0.1
       }
    }
   
   object { /* Mouth */
      intersection {
         sphere { <0.0  0.0  0.0>  28.0  }
         union {
            plane { <0.0 2.0 0.0> -2.0 rotate <@lip_lower@ 0.0 0.0>  }
            plane { <0.0 -2.0 0.0> -2.0 rotate <@lip_upper@ 0.0 0.0>  }
          }
       }
   
      texture {
         ambient 0.25
         diffuse 0.75
         colour red 0.5
       }
    }
   
   object { /* Tongue */
      union {
         sphere { <3.0 0.0 -15.0> 10.0  }
         sphere { <-3.0 0.0 -15.0> 10.0  }
       }
      rotate <-45.0 0.0 0.0>

      texture {
         colour red 1.0
         wrinkles 0.5
         scale <0.5 0.5 0.5>
         ambient 0.3
         diffuse 0.7
         reflection 0.5
       }
    }

   composite { eye        /* Right eye */
      rotate <-5.0 -15.0 @r_eye_dir@>
      translate <-8.0 +10.988 -23.564>
    }

   composite { eye        /* Left eye */
      rotate <-5.0 15.0 @l_eye_dir@>
      translate <+8.0 +10.988 -23.564>
    }

   object {  /* Right Eye-brow */
      quadric { Ellipsoid  }
      scale <9.0 2.0 9.0>
      translate <0.0 8.0 6.0>
      rotate <0.0 0.0 @r_eyebrow_angle@>
      rotate <-25.0 -20.0 0.0>
      translate <0.0 0.0 -26.0>
      rotate <25.0 20.0 0.0>
      texture {
         colour red 0.0 green 0.0 blue 0.0
       }
    }
   
   object { /* Left Eye-brow */
      quadric { Ellipsoid  }
      scale <9.0 2.0 9.0>
      translate <0.0 8.0 6.0>
      rotate <0.0 0.0 @l_eyebrow_angle@>
      rotate <-25.0 20.0 0.0>
      translate <0.0 0.0 -26.0>
      rotate <25.0 -20.0 0.0>
      texture {
         colour red 0.0 green 0.0 blue 0.0
      }
   }

   bounded_by {
      sphere { <0.0 0.0 -2.0> 32.0  }
   }

   rotate <0 @pac_dir@ 0>
   translate <@pac_x@ 32.0 @pac_z@>
}

&f1_exists
object { food
   translate <@food_x@ 27.0 @food1_z@>
}
&

&f2_exists
object { food
   translate <@food_x@ 27.0 @food2_z@>
}
&

&f3_exists
object { food
   translate <@food_x@ 27.0 @food3_z@>
}
&

&f4_exists
object { food
   translate <@food_x@ 27.0 @food4_z@>
}
&

&f5_exists
object { food
   translate <@food_x@ 27.0 @food5_z@>
}
&

&f6_exists
object { food
   translate <@food_x@ 27.0 @food6_z@>
}
&

&f7_exists
object { food
   translate <@food_x@ 27.0 @food7_z@>
}
&

&f8_exists
object { food
   translate <@food_x@ 27.0 @food8_z@>
}
&

object {   /* Light source */
   union {
      sphere {
         <60.0 120.0 -170.0> 2.0
         texture { color White ambient 1 diffuse 0 }
      }
      light_source { <60.0 120.0 -170.0> color White }
   }
   no_shadow
}
 

