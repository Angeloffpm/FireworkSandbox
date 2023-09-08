#include "CSCIx229.h"

//  2D vector
typedef struct {float x,y;} vec2;
typedef struct {float x,y,z;} vec3;

//  Make vec2 unit length
vec2 normalize(float x,float y)
{
   float len = sqrt(x*x+y*y);
   if (len>0)
   {
      x /= len;
      y /= len;
   }
   return (vec2){x,y};
}

#define np 10
vec2 cannon_outline[np] = 
{
    {0.000, 0.000},
    {0.125, 0.025},
    {0.220, 0.100},
    {0.230, 0.200},
    // Begin Texture
    {0.220, 0.300},
    {0.200, 0.420},
    {0.180, 0.550},
    {0.170, 0.650},
    {0.160, 0.780},
    {0.150, 1.000}
};

vec2 cannon_norms[np];

void CalcNorm(void)
{
    //  Calculate normals for each facet
    //  y increases with point index so dy>0
    for (int i=0;i<np-2;i++)
    {
        // Vector in the plane of the facet
        float dx = cannon_outline[i+1].x - cannon_outline[i].x;
        float dy = cannon_outline[i+1].y - cannon_outline[i].y;
        // Normal is perpendicular
        // dy>0 so normal faces out
        cannon_norms[i] = normalize(dy,-dx);
    }
    cannon_norms[np-1] = cannon_norms[np-2];

    //  Average normals of adjacent facets
    //  First and last normal unchanged
    vec2 N2 = cannon_norms[0];
    for (int i=1;i<np;i++)
    {
        vec2 N1 = N2;
        N2 = cannon_norms[i];
        cannon_norms[i] = normalize(N1.x+N2.x , N1.y+N2.y);
        // printf("X:%f Z:%f\n", cannon_norms[i].x, cannon_norms[i].y);
    }
    cannon_norms[np-1] = (vec2){1.0,0.0};
}

static void cube(double x,double y,double z,
                 double dx,double dy,double dz,
                 double r, double g, double b)
{
   //  Set specular color to white
   float white[] = {1,1,1,1};
   float black[] = {0,0,0,1};
   float shiny =32;
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);
   //  Save transformation
   glPushMatrix();
   //  Offset, scale and rotate
   glTranslated(x,y,z);
   glScaled(dx,dy,dz);
   //  Cube
   glBegin(GL_QUADS);
   //  Front
   glColor3f(r,g,b);
   glNormal3f( 0, 0, 1);
   glVertex3f(-1,-1, 1);
   glVertex3f(+1,-1, 1);
   glVertex3f(+1,+1, 1);
   glVertex3f(-1,+1, 1);
   //  Back
   glNormal3f( 0, 0,-1);
   glVertex3f(+1,-1,-1);
   glVertex3f(-1,-1,-1);
   glVertex3f(-1,+1,-1);
   glVertex3f(+1,+1,-1);
   //  Right
   glNormal3f(+1, 0, 0);
   glVertex3f(+1,-1,+1);
   glVertex3f(+1,-1,-1);
   glVertex3f(+1,+1,-1);
   glVertex3f(+1,+1,+1);
   //  Left
   glNormal3f(-1, 0, 0);
   glVertex3f(-1,-1,-1);
   glVertex3f(-1,-1,+1);
   glVertex3f(-1,+1,+1);
   glVertex3f(-1,+1,-1);
   //  Top
   glNormal3f( 0,+1, 0);
   glVertex3f(-1,+1,+1);
   glVertex3f(+1,+1,+1);
   glVertex3f(+1,+1,-1);
   glVertex3f(-1,+1,-1);
   //  Bottom
   glNormal3f( 0,-1, 0);
   glVertex3f(-1,-1,-1);
   glVertex3f(+1,-1,-1);
   glVertex3f(+1,-1,+1);
   glVertex3f(-1,-1,+1);
   //  End
   glEnd();
   //  Undo transofrmations
   glPopMatrix();
}

/* Wood/textured cuboid */
static void woodcube(double x, double y, double z,
                     double dx, double dy, double dz,
                     unsigned int tex)
{
    float white[] = {1,1,1,1};
    float black[] = {0,0,0,1};
    float shiny =32;
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
    glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);

    glPushMatrix();
    glTranslated(x,y,z);
    glScaled(dx,dy,dz);

    glColor3f(1,1,1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
    glBegin(GL_QUADS);
    // Front face
    glNormal3f(0,0,1);
    glTexCoord2d(0,0);  glVertex3f(-1,-1,1);
    glTexCoord2d(dx,0); glVertex3f(+1,-1,1);
    glTexCoord2d(dx,dy);glVertex3f(+1,+1,1);
    glTexCoord2d(0,dy); glVertex3f(-1,+1,1);
    // Right face
    glNormal3f(1,0,0);
    glTexCoord2d(0,0);  glVertex3f(+1,-1,+1);
    glTexCoord2d(dx,0); glVertex3f(+1,-1,-1);
    glTexCoord2d(dx,dy);glVertex3f(+1,+1,-1);
    glTexCoord2d(0,dy); glVertex3f(+1,+1,+1);
    // Back face
    glNormal3f(0,0,-1);
    glTexCoord2d(0,0);  glVertex3f(+1,-1,-1);
    glTexCoord2d(dx,0); glVertex3f(-1,-1,-1);
    glTexCoord2d(dx,dy);glVertex3f(-1,+1,-1);
    glTexCoord2d(0,dy); glVertex3f(+1,+1,-1);
    // Left face
    glNormal3f(-1,0,0);
    glTexCoord2d(0,0);  glVertex3f(-1,-1,-1);
    glTexCoord2d(dx,0); glVertex3f(-1,-1,+1);
    glTexCoord2d(dx,dy);glVertex3f(-1,+1,+1);
    glTexCoord2d(0,dy); glVertex3f(-1,+1,-1);
    // Top face
    glNormal3f(0,1,0);
    glTexCoord2d(0,0);  glVertex3f(-1,+1,+1);
    glTexCoord2d(dx,0); glVertex3f(+1,+1,+1);
    glTexCoord2d(dx,dy);glVertex3f(+1,+1,-1);
    glTexCoord2d(0,dy); glVertex3f(-1,+1,-1);
    // Bottom face
    glNormal3f(0,-1,0);
    glTexCoord2d(0,0);  glVertex3f(-1,-1,+1);
    glTexCoord2d(dx,0); glVertex3f(+1,-1,+1);
    glTexCoord2d(dx,dy);glVertex3f(+1,-1,-1);
    glTexCoord2d(0,dy); glVertex3f(-1,-1,-1);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

/*
 * Helper function for the Cannon. Draws the barrel
*/
static void cannon_barrel(double x, double y, double z,
                          double turn, double elev,
                          unsigned int texture)
{
    glPushMatrix();
    glTranslated(0,0.3,0);
    glRotated(elev, 1,0,0);

    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D,texture);

    glColor3f(1,1,1);
    for (int i = 0; i < np-1; i++)
    {
        glBegin(GL_QUAD_STRIP);
        for (int th = 0; th <= 360; th += 15)
        {
            float c = Cos(th);
            float s = Sin(th);

            glNormal3f(c*cannon_norms[i].x, s*cannon_norms[i].x, cannon_norms[i].y);
            glTexCoord2d(th/360.0, cannon_outline[i].y/cannon_outline[np-1].y);
            glVertex3f(c*cannon_outline[i].x, s*cannon_outline[i].x, cannon_outline[i].y);

            glNormal3f(c*cannon_norms[i+1].x, s*cannon_norms[i+1].x, cannon_norms[i+1].y);
            glTexCoord2d(th/360.0, cannon_outline[i+1].y/cannon_outline[np-1].y);
            glVertex3f(c*cannon_outline[i+1].x, s*cannon_outline[i+1].x, cannon_outline[i+1].y);

        }
        glEnd();
    }
    glDisable(GL_TEXTURE_2D);

    glColor3f(0,0,0);
    float black[] = {0,0,0,1};
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,black);
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0,0,1);
    glVertex3f(0,0,cannon_outline[np-1].y-0.05);
    for (int th = 0; th <= 360; th += 15)
    {
        double r = cannon_outline[np-1].x;
        glVertex3f(Cos(th)*r, Sin(th)*r, cannon_outline[np-1].y-0.05);
    }
    glEnd();
    float white[] = {1,1,1,1};
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
    glPopMatrix();
}

/* 
 * Helper function for the Cannon. Draws the bases
 * mirror flips over the y axis
 */
static void cannon_base(int mirror)
{
    glColor3f(1,0,0);
    // Inside face
    glBegin(GL_QUADS);
    glNormal3f(mirror*-1,0,0);
    glVertex3f(mirror*0.2,0,0.15); glVertex3f(mirror*0.2,0,-0.15);
    glVertex3f(mirror*0.2,0.3,-0.15); glVertex3f(mirror*0.2,0.3,0.15);
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(mirror*0.2,0.3,0);
    for (int theta = 0; theta <= 180; theta += 10)
    {
        glVertex3f(mirror*0.2, 0.3+Sin(theta)*0.1, Cos(theta)*-0.15);
    }
    glEnd();
    // Outside face
    glBegin(GL_QUADS);
    glNormal3f(mirror*1,0,0);
    glVertex3f(mirror*0.4,0,0.15); glVertex3f(mirror*0.4,0,-0.15);
    glVertex3f(mirror*0.4,0.3,-0.15); glVertex3f(mirror*0.4,0.3,0.15);
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    for (int theta = 0; theta <= 180; theta += 10)
    {
        glVertex3f(mirror*0.4, 0.3+Sin(theta)*0.1, Cos(theta)*-0.15);
    }
    glEnd();
    // Connect faces
    glBegin(GL_QUAD_STRIP);
    glNormal3f(0,-1,0);
    glVertex3f(mirror*0.4,0,0.15); glVertex3f(mirror*0.2,0,0.15);
    glVertex3f(mirror*0.4,0,-0.15); glVertex3f(mirror*0.2,0,-0.15);
    for (int theta = 0; theta <= 180; theta += 10)
    {
        glNormal3f(0,Sin(theta),-Cos(theta));
        glVertex3f(mirror*0.4,0.3+Sin(theta)*0.1,Cos(theta)*-0.15);
        glVertex3f(mirror*0.2,0.3+Sin(theta)*0.1,Cos(theta)*-0.15);
    }
    glVertex3f(mirror*0.4,0,0.15);
    glVertex3f(mirror*0.2,0,0.15);
    glEnd();
}

/*
 * Draws a cannon at (x, y, z)
 *  with rotation turn (turn angle)
 *  tilted elev (elevation angle)
 *  with cannon texture (texture)
 */
static void cannon(double x, double y, double z,
                   double turn, double elev,
                   unsigned int texture)
{
    // Lighting & Materials
    float white[] = {1,1,1,1};
    float shiny = 32;
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);

    glPushMatrix();
    glTranslated(x,y,z);
    glRotated(turn, 0,1,0);

    // Draw bases
    cannon_base(1);
    cannon_base(-1);

    // Draw barrel
    cannon_barrel(x,(y+0.3),z, turn, elev, texture);

    // Restore previous transformations
    glPopMatrix();
}

/*
 * Draw a firework rocket
 *  at (x, y, z)
 *  with cannon turn ct and cannon elevation ce
 */
static void rocket(double x, double y, double z,
                   double ct, double ce)
{
    // Measurements
    const double ch = 0.3;  // Cone height
    const double cr = 0.25; // Cone radius
    // const double bh = 0.5;  // Base height
    const double br = 0.15; // Base radius
    
    // Save transformations
    glPushMatrix();
    // Transformations
    glTranslated(x,y,z);
    // glRotated(90, 1,0,0);
    glRotated(ct, 0,1,0);
    glRotated(ce, 1,0,0);

    // Draw stick
    cube(-0.1,0,0.2, 0.05,0.05,0.2, 0.5,0.3,0.2);

    // Draw cyclindrical base
    glColor3f(1,1,1);
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0,-1,0);
    glVertex3f(0,0,0.4);
    for (int theta = 0; theta <= 360; theta += 15)
    {
        glVertex3f(br*Cos(theta),br*Sin(theta),0.4);
    }
    glEnd();
    glBegin(GL_QUAD_STRIP);
    for (int theta = 0; theta <= 360; theta += 15)
    {
        glNormal3f(Cos(theta),Sin(theta),0);
        glVertex3f(br*Cos(theta),br*Sin(theta),0.4); glVertex3f(br*Cos(theta),br*Sin(theta),0.9);
    }
    glEnd();

    // Draw nose cone
    glColor3f(1,0,0);
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0,-1,0);
    glVertex3f(0,0,0.9);
    for (int theta = 0; theta <= 360; theta += 15)
    {
        glVertex3f(cr*Cos(theta),cr*Sin(theta),0.9);
    }
    glEnd();
    glBegin(GL_QUAD_STRIP);
    for (int theta = 0; theta <= 360; theta += 15)
    {
        glNormal3f(ch*Cos(theta),ch*Sin(theta),cr);
        glVertex3f(0,0,0.9+ch);
        glVertex3f(cr*Cos(theta),cr*Sin(theta),0.9);
    }
    glEnd();

    // Restore transformations
    glPopMatrix();

}



/*
 *  Draw a ball
 *     at (x,y,z)
 *     radius r
 */
static void ball(double x,double y,double z,double r)
{
   //  Save transformation
   glPushMatrix();
   //  Offset, scale and rotate
   glTranslated(x,y,z);
   glScaled(r,r,r);
   //  White ball
   glutSolidSphere(1.0,16,16);
   //  Undo transofrmations
   glPopMatrix();
}

/*
 * Helper function that draws a beam
 * from point A to B for the tower
*/
static void beam(double x1, double y1, double z1,
                 double x2, double y2, double z2,
                 double r, double texture)
{
    vec3 verts[8] = {
        {x1+r,y1,z1+r},
        {x1-r,y1,z1+r},
        {x1-r,y1,z1-r},
        {x1+r,y1,z1-r},
        {x2+r,y2,z2+r},
        {x2-r,y2,z2+r},
        {x2-r,y2,z2-r},
        {x2+r,y2,z2-r}
    };
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,texture);
    glBegin(GL_QUADS);
    // Bottom face
    glNormal3f(0,-1,0);
    glTexCoord2d(0.0,0.0); glVertex3f(verts[0].x, verts[0].y, verts[0].z);
    glTexCoord2d(0.1,0.0); glVertex3f(verts[1].x, verts[1].y, verts[1].z);
    glTexCoord2d(0.1,0.1); glVertex3f(verts[2].x, verts[2].y, verts[2].z);
    glTexCoord2d(0.0,0.1); glVertex3f(verts[3].x, verts[3].y, verts[3].z);
    glColor3f(1,1,1);
    // glEnable(GL_NORMALIZE);
    // Side faces
    double dy = y2-y1;
    double dx = verts[0].x - verts[4].x;
    for (int i = 0; i < 4; i++)
    {
        switch(i)
        {
            case 0:
                glNormal3f(0,0,1);
                break;
            case 1:
                glNormal3f(-dy,dx,0);
                break;
            case 2:
                glNormal3f(0,0,-1);
                break;
            case 3:
                glNormal3f(dy,-dx,0);
                break;
        };
        glTexCoord2d(0.1*i,0); glVertex3f(verts[i].x,      verts[i].y,     verts[i].z);
        glTexCoord2d(0.1*i,1); glVertex3f(verts[i+4].x,    verts[i+4].y,   verts[i+4].z);
        if(i < 3)
        {
            glTexCoord2d(0.1*(i+1),1); glVertex3f(verts[i+5].x, verts[i+5].y, verts[i+5].z);
            glTexCoord2d(0.1*(i+1),0); glVertex3f(verts[i+1].x, verts[i+1].y, verts[i+1].z);
        }
        else
        {
            glTexCoord2d(0.1*(i+1),1); glVertex3f(verts[4].x, verts[4].y, verts[4].z);
            glTexCoord2d(0.1*(i+1),0); glVertex3f(verts[0].x, verts[0].y, verts[0].z);
        }

    }
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

/*
 * Draw the tower on which the cannon sits
*/
static void tower(double x, double y, double z,
                  double h, unsigned int texture)
{
    // Measurements
    const double rad        = 1; // platform radius
    const double height     = h; // tower height
    // Save transformations
    glPushMatrix();
    // Apply transformations
    glTranslated(x,y,z);

    // Draw platform
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,texture);
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(1,1,1);
    glNormal3f(0,1,0); glTexCoord2d(0.5,0.5); glVertex3f(0,height,0);
    for (int theta = 0; theta <= 360; theta += 15)
    {
        glNormal3f(0,1,0); glTexCoord2d(Cos(theta)/2+0.5,Sin(theta)/2+0.5);
        glVertex3f(Cos(theta)*rad,height,Sin(theta)*rad);
    }
    glEnd();
    glBegin(GL_QUAD_STRIP);
    for (int theta = 0; theta <= 360; theta += 15)
    {
        glNormal3f(Cos(theta),0,Sin(theta)); glTexCoord2d(theta/120.0, 0.2);
        glVertex3f(Cos(theta)*rad,height,Sin(theta)*rad);
        glNormal3f(Cos(theta),0,Sin(theta)); glTexCoord2d(theta/120.0, 0.0);
        glVertex3f(Cos(theta)*rad,height-0.1,Sin(theta)*rad);
    }
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0,-1,0); glTexCoord2d(0.5,0.5); glVertex3f(0,height,0);
    for (int theta = 0; theta <= 360; theta += 15)
    {
        glNormal3f(0,-1,0); glTexCoord2d(Cos(theta)/2+0.5,Sin(theta)/2+0.5);
        glVertex3f(Cos(theta)*rad,height-0.1,Sin(theta)*rad);
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Support beams
    glPushMatrix();
    for (int theta = 0; theta < 360; theta += 90)
    {
        beam(rad/2,0,0, -rad+0.2,height-0.1,0, 0.1, texture);
        glRotated(90, 0,1,0);
    }
    glPopMatrix();

    // Reapply transformations
    glPopMatrix();
}

static void fwheel(double x, double y, double z,
                   double turn)
{
    const double outrad = 5.2;
    const double inrad = 4.8;
    const double half = (outrad + inrad) / 2;
    const float  zi[] = {+1,+0.9,-0.9,-1};

    glPushMatrix();
    glTranslated(x,y,z);
    glRotated(turn, 0,0,1);

    // Outer framing
    for (int i = 0; i < 4; i++)
    {
        glBegin(GL_QUAD_STRIP);
        for (int theta = 0; theta <= 360; theta += 15)
        {
            double costh = Cos(theta); double sinth = Sin(theta);
            int zn = -1;
            if (i % 2 == 0)
                zn = 1;
            glNormal3f(0,0,zn); glVertex3d(costh*outrad,sinth*outrad,zi[i]);
            glNormal3f(0,0,zn); glVertex3d(costh*inrad ,sinth*inrad ,zi[i]);
        }
        glEnd();
    }
    for (int theta = 0; theta <= 360; theta += 30)
    {
        cube(Cos(theta)*half,Sin(theta)*half,0, 0.1,0.1,0.9, 1,1,1);
    }
    for (int i = 0; i < 3; i+= 2)
    {
        // outside faces
        glBegin(GL_QUAD_STRIP);
        for (int theta = 0; theta <= 360; theta += 15)
        {
            glNormal3f(Cos(theta),Sin(theta),0); glVertex3d(Cos(theta)*outrad,Sin(theta)*outrad,zi[i]);
            glNormal3f(Cos(theta),Sin(theta),0); glVertex3d(Cos(theta)*outrad,Sin(theta)*outrad,zi[i+1]);
        }
        glEnd();
        // inside faces
        glBegin(GL_QUAD_STRIP);
        for (int theta = 0; theta <= 360; theta += 15)
        {
            glNormal3f(-Cos(theta),-Sin(theta),0); glVertex3d(Cos(theta)*inrad,Sin(theta)*inrad,zi[i]);
            glNormal3f(-Cos(theta),-Sin(theta),0); glVertex3d(Cos(theta)*inrad,Sin(theta)*inrad,zi[i+1]);
        }
        glEnd();
    }
    // Middle to outer framing
    glPushMatrix();
    for (int theta = 0; theta < 180; theta += 30)
    {
        glRotated(30, 0,0,1);
        cube(0,0,+0.90, half,0.075,0.075, 1,1,1);
        cube(0,0,-0.90, half,0.075,0.075, 1,1,1);
    }
    glPopMatrix();
    // Center connector
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0,0,1); glVertex3f(0,0,zi[0]);
    for (int theta = 0; theta <= 360; theta += 15)
    {
        glNormal3f(0,0,1); glVertex3f(Cos(theta),Sin(theta),zi[0]+0.1);
    }
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0,0,-1); glVertex3f(0,0,zi[3]);
    for (int theta = 0; theta <= 360; theta += 15)
    {
        glNormal3f(0,0,-1); glVertex3f(Cos(theta),Sin(theta),zi[3]-0.1);
    }
    glEnd();
    glBegin(GL_QUAD_STRIP);
    for (int theta = 0; theta <= 360; theta += 15)
    {
        glNormal3f(Cos(theta),Sin(theta),0); glVertex3f(Cos(theta),Sin(theta),zi[0]+0.1);
        glNormal3f(Cos(theta),Sin(theta),0); glVertex3f(Cos(theta),Sin(theta),zi[3]-0.1);
    }
    glEnd();
    // Lights
    glDisable(GL_LIGHTING);
    for (int theta = 0; theta < 360; theta += 3)
    {
        if (theta % 60 < 30)
            glColor3f(1,1,0);
        else
            glColor3f(1,0.2,0);
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(half*Cos(theta),half*Sin(theta),-1.01);
        for (int th = 0; th <= 360; th += 60)
        {
            glVertex3f((half*Cos(theta))+(Cos(th)*0.1),(half*Sin(theta))+(Sin(th)*0.1),-1.01);
        }
        glEnd();
    }
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

/*
 * Draw a ferris wheel
*/
static void ferris(double x, double y, double z,
                   double turn)
{
    const double outrad = 5.2;
    // const double inrad = 4.8;
    // const double half = (outrad + inrad) / 2;
    // const float  zi[] = {+1,+0.9,-0.9,-1};
    glPushMatrix();
    glColor3f(1,1,1);

    fwheel(x,y+outrad,z, turn);

    glPopMatrix();
}

static void seat(double x, double y, double z,
                 double dx, double dy, double dz,
                 unsigned int woodtex, unsigned int metaltex)
{
    glPushMatrix();
    glTranslated(x,y,z);
    glScaled(dx,dy,dz);
    woodcube(0,-0.4,0, 0.1,0.5,0.2, metaltex);
    woodcube(0.25,-0.85,0, 0.3,0.1,0.6, woodtex);

    glPopMatrix();
}

/*
 * Draw a tent
*/
static void tent(double x, double y, double z,
                 double dx, double dy, double dz,
                 double turn,
                 double r1, double g1, double b1,
                 double r2, double g2, double b2,
                 unsigned int opening, unsigned int ripples)
{
    const int panel = 30;   // Angle of each panel
    float rads[5] = {1, 0.9, 0.95, 0.3, 0};
    float angs[5] = {0.2, 0, -0.1, 1, 0.5};
    int color = 0;
    // Transformations
    glPushMatrix();

    glTranslated(x,y,z);
    glRotated(turn,0,1,0);
    glScaled(dx,dy,dz);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, ripples);

    for (int i = 0; i < 4; i++)
    {
        for (int theta = 0; theta < 360; theta += panel)
        {
            if (theta == 0 && i == 0)
                glBindTexture(GL_TEXTURE_2D, opening);
            else
                glBindTexture(GL_TEXTURE_2D, ripples);
            if (color)
                glColor3f(r1,g1,b1);
            else
                glColor3f(r2,g2,b2);
            color = 1 - color;
            double rad = rads[i]; double costh = Cos(theta); double sinth = Sin(theta);
            glBegin(GL_QUADS);
            glNormal3f(costh,angs[i],sinth);
            glTexCoord2d(0,0);
            glVertex3f(rad*costh,0.2*i,rad*sinth);
            glNormal3f(costh,angs[i+1],sinth);
            glTexCoord2d(0,1);
            glVertex3f(rads[i+1]*costh,0.2*(i+1),rads[i+1]*sinth);
            glNormal3f(Cos(theta+panel),angs[i+1],Sin(theta+panel));
            glTexCoord2d(1,1);
            glVertex3f(rads[i+1]*Cos(theta+panel),0.2*(i+1),rads[i+1]*Sin(theta+panel));
            glNormal3f(Cos(theta+panel),angs[i],Sin(theta+panel));
            glTexCoord2d(1,0);
            glVertex3f(rad*Cos(theta+panel),0.2*i,rad*Sin(theta+panel));
            glEnd();
        }
    }
    glDisable(GL_TEXTURE_2D);
    
    glPopMatrix();
}

/* Kiosk */
static void kiosk(double x, double y, double z,
                  double dx, double dy, double dz,
                  double turn,
                  unsigned int woodtex, unsigned int fabrictex,
                  double r, double g, double b)
{

    glPushMatrix();

    glTranslated(x,y,z);
    glRotated(turn, 0,1,0);
    glScaled(dx,dy,dz);

    woodcube(0,0.25,0.9, 1,0.25,0.1, woodtex); // Front
    woodcube(0.9,0.25,0, 0.1,0.25,1, woodtex);  // Right side
    woodcube(-0.9,0.25,0, 0.1,0.25,1, woodtex); // Left side
    woodcube(0,1,-0.9, 1,1,0.1, woodtex); // Back
    woodcube(0.9,0.8,0.9, 0.05,0.8,0.05, woodtex); // Support beams
    woodcube(-0.9,0.8,0.9, 0.05,0.8,0.05, woodtex);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, fabrictex);
    glColor3f(r,g,b);
    glBegin(GL_QUADS);
    glNormal3f(0,1,0.3);
    glTexCoord2d(0,0); glVertex3d(-1,1.6,1);
    glTexCoord2d(0,dy); glVertex3d(-1,2.05,-1);
    glTexCoord2d(dx,dy); glVertex3d(1,2.05,-1);
    glTexCoord2d(dx,0); glVertex3d(1,1.6,1);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();

}

/* Basic tree function */
static void tree(double x, double y, double z,
                 double dx, double dy, double dz,
                 double dth, unsigned int woodtex,
                 unsigned int leaftex)
{
    float trunkrads[] = {0.4, 0.35, 0.35};
    float leafrads[3][3]  = {{2, 1, 0.6}, {1.5, 0.9, 0.8}, {1.2, 0.7, 0.6}};
    float normals[] = {0.6,0.3,0};

    float green[] = {0,0.6,0,1};
    float brown[] = {0.5,0.3,0};

    glPushMatrix();
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,2);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,brown);

    glTranslated(x,y,z);
    glScaled(dx,dy,dz);

    // Trunk
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, woodtex);
    glColor3f(0.6,0.4,0);
    glBegin(GL_QUAD_STRIP);
    for (int theta = 0; theta <= 360; theta += dth)
    {
        glNormal3f(Cos(theta),0.1,Sin(theta));  glTexCoord2d(theta/60.0, 0);
        glVertex3f(Cos(theta)*trunkrads[0],0,Sin(theta)*trunkrads[0]);
        glNormal3f(Cos(theta),0,Sin(theta));    glTexCoord2d(theta/60.0, 0.5);
        glVertex3f(Cos(theta)*trunkrads[1],0.5,Sin(theta)*trunkrads[1]);
    }
    glEnd();
    glBegin(GL_QUAD_STRIP);
    for (int theta = 0; theta <= 360; theta += dth)
    {
        glNormal3f(Cos(theta),0,Sin(theta));    glTexCoord2d(theta/60.0, 0);
        glVertex3f(Cos(theta)*trunkrads[1],0,Sin(theta)*trunkrads[1]);
        glNormal3f(Cos(theta),0,Sin(theta));    glTexCoord2d(theta/60.0, 0.5);
        glVertex3f(Cos(theta)*trunkrads[2],1,Sin(theta)*trunkrads[2]);
    }
    glEnd();
    // Leaves
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,green);
    glBindTexture(GL_TEXTURE_2D, leaftex);
    glColor3f(1,1,1);
    double height = 1;
    for (int i = 0; i < 3; i++) // 3 tiers of leaves
    {
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0,-1,0); glTexCoord2d(0.5,0.5);
        glVertex3f(0,height,0);
        for (int theta = 0; theta <= 360; theta += dth)
        {
            glTexCoord2d(Cos(theta),Sin(theta));
            glVertex3f(Cos(theta)*leafrads[i][0],height,Sin(theta)*leafrads[i][0]);
        }
        glEnd();
        glBegin(GL_QUAD_STRIP);
        for (int theta = 0; theta <= 360; theta += dth)
        {
            glNormal3f(Cos(theta),normals[0],Sin(theta));   glTexCoord2d(theta/60.0, 0);
            glVertex3f(Cos(theta)*leafrads[i][0],height,Sin(theta)*leafrads[i][0]);
            glNormal3f(Cos(theta),normals[1],Sin(theta));   glTexCoord2d(theta/60.0, 0.5);
            glVertex3f(Cos(theta)*leafrads[i][1],height+0.5,Sin(theta)*leafrads[i][1]);
        }
        glEnd();
        height += 0.5;
        glBegin(GL_QUAD_STRIP);
        for (int theta = 0; theta <= 360; theta += dth)
        {
            glNormal3f(Cos(theta),normals[1],Sin(theta));   glTexCoord2d(theta/60.0, 0);
            glVertex3f(Cos(theta)*leafrads[i][1],height,Sin(theta)*leafrads[i][1]);
            glNormal3f(Cos(theta),normals[2],Sin(theta));   glTexCoord2d(theta/60.0, 0.5);
            glVertex3f(Cos(theta)*leafrads[i][2],height+0.5,Sin(theta)*leafrads[i][2]);
        }
        glEnd();
        height += 0.5;
    }
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0,1,0); glTexCoord2d(0.5,0.5);
    glVertex3f(0,height+1,0);
    for (int theta = 0; theta <= 360; theta += dth)
    {
        glNormal3f(Cos(theta),0,Sin(theta)); glTexCoord2d(Cos(theta),Sin(theta));
        glVertex3f(Cos(theta)*leafrads[2][2],height,Sin(theta)*leafrads[2][2]);
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}



static void groundtile(double x, double y, double z, double s,
                       unsigned int tex)
{
    glPushMatrix();
    float black[] = {0,0,0,1};
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,0);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,black);
    glTranslated(x,y,z);
    glScaled(s,s,s);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
    glBegin(GL_QUADS);
    glNormal3f(0,1,0); glTexCoord2d(0,0); glVertex3f(0,0,0);
    glNormal3f(0,1,0); glTexCoord2d(1,0); glVertex3f(1,0,0);
    glNormal3f(0,1,0); glTexCoord2d(1,1); glVertex3f(1,0,1);
    glNormal3f(0,1,0); glTexCoord2d(0,1); glVertex3f(0,0,1);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

static void skybox(unsigned int texture, double D)
{
    glPushMatrix();
    glScaled(D,D,D);
    glEnable(GL_TEXTURE_2D);
    glColor3f(1,1,1);

    //  Sides
    glBindTexture(GL_TEXTURE_2D,texture);
    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex3f(-1,-1,-1);
    glTexCoord2f(1,0); glVertex3f(+1,-1,-1);
    glTexCoord2f(1,4); glVertex3f(+1,+1,-1);
    glTexCoord2f(0,4); glVertex3f(-1,+1,-1);

    glTexCoord2f(0,0); glVertex3f(+1,-1,-1);
    glTexCoord2f(1,0); glVertex3f(+1,-1,+1);
    glTexCoord2f(1,4); glVertex3f(+1,+1,+1);
    glTexCoord2f(0,4); glVertex3f(+1,+1,-1);

    glTexCoord2f(0,0); glVertex3f(+1,-1,+1);
    glTexCoord2f(1,0); glVertex3f(-1,-1,+1);
    glTexCoord2f(1,4); glVertex3f(-1,+1,+1);
    glTexCoord2f(0,4); glVertex3f(+1,+1,+1);

    glTexCoord2f(0,0); glVertex3f(-1,-1,+1);
    glTexCoord2f(1,0); glVertex3f(-1,-1,-1);
    glTexCoord2f(1,4); glVertex3f(-1,+1,-1);
    glTexCoord2f(0,4); glVertex3f(-1,+1,+1);

    //  Top and bottom
    glTexCoord2f(0,0); glVertex3f(+1,+1,-1);
    glTexCoord2f(1,0); glVertex3f(+1,+1,+1);
    glTexCoord2f(1,4); glVertex3f(-1,+1,+1);
    glTexCoord2f(0,4); glVertex3f(-1,+1,-1);

    glTexCoord2f(1,4); glVertex3f(-1,-1,+1);
    glTexCoord2f(0,4); glVertex3f(+1,-1,+1);
    glTexCoord2f(0,0); glVertex3f(+1,-1,-1);
    glTexCoord2f(1,0); glVertex3f(-1,-1,-1);
    glEnd();

    //  Undo
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

}