/*
 * Pierce Angeloff
 * CSCI 4229 Final - Carnival Firework Sandbox
 * 
 * Controls
 * === Game mode ===
 * p            Toggle game mode on/off
 * 0            Reset cannon turn & elevation
 * WASD         Move cannon up/left/down/right
 * Space        Fire a firework
 * r/R          Decrease/increase the red value of the firework
 * g/G          Decrease/increase the green value of the firework
 * b/B          Decrease/increase the blue value of the firework
 * === Spectator mode ===
 * (You can still control the cannon and fireworks in spectator mode)
 * p            Toggle spectator mode on/off
 * 0            Reset view angles
 * PGUP/PGDN    Decrase/increase world/size (dim)
 * === Universal controls ===
 * ESC      Exit
 * f        Toggle fog
*/

#include "CSCIx229.h"
#include "cannon.c"

/* Globals */
double dim      = 25;       // Size of world
double asp      = 1;        // Aspect ratio
int axes        = 0;        // Toggle axes
unsigned int texture[9];    // Textures
unsigned int sky[1];        // Skybox
double fturn    = 0;        // Ferris wheel turn
// Lighting
int lighting    = 1;    // Toggle lighting
int ambient     = 0;    // Ambient lighting
int diffuse     = 20;   // Diffuse lighting
int specular    = 20;   // Specular lighting
int zh          = 90;   // Light azimuth
int fog         = 1;    // Fog toggle
// Camera
double camx = 0;
double camy = 0;
double camz = 0;
int th      = 0;
int ph      = 0;
// Game variables
int game                    = 1;    // Toggle game mode
int cannon_turn             = 0;    // Cannon turn angle
int cannon_elev             = 0;    // Cannon elevation angle
double cx                   = 0;    // Cannon x-coord
double cy                   = 2;    // Cannon x-coord
double cz                   = 0;    // Cannon x-coord
const int speed             = 20;   // Speed of firework (higher = slower)
const int flighttime        = 3000; // Flighttime of firework in milliseconds
const float colorchange    = 0.02; // Increment/Decrement of color change
typedef struct 
{
    float x,y,z;
    float dx,dy,dz;
    int ft,fe;
    int active;
    double time;
    double r,g,b;
} firework;
firework fw = {0,0,0, 0,0,0, 0,0, 0, 0, 1,1,1}; // Firework variable, stores all data
typedef struct
{
    int active;
    double time;
    double x,y,z;
    double r,g,b;
} fwlight;
double fwintense = 1.0;
fwlight fwl = {0, 0, 0,0,0, 1,1,1};             // Firework's light
typedef struct
{
    double x,y,z;
    double dx,dy,dz;
    double r,g,b;
} fwparticle;
double partscale = 0.1;
#define parts 50
fwparticle particles[parts];                    // Firework particles
// Tree dimensions randomly generated at startup:
const double tree_distance = 40;
const int num_trees = 30;
const int tree_angle = 360 / num_trees;
vec3 trees[30];
static void generateTrees()
{
    for (int i = 0; i < num_trees; i++)
    {
        trees[i].x = 2.5 + (2.0 * (double)rand() / (double)RAND_MAX);
        trees[i].y = 2.5 + (2.0 * (double)rand() / (double)RAND_MAX);
        trees[i].z = 2.5 + (2.0 * (double)rand() / (double)RAND_MAX);
    }
}

/* 
 * GLUT calls this to display the scene
*/
void display()
{
    // Erase window and buffers, enable z-buffer
    // glClearColor(3/255.0, 87/255.0, 135/255.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // Fog
    if (fog)
    {
        glEnable(GL_FOG);
        GLfloat fogcolor[] = {0,0,0,1};
        glFogf(GL_FOG_MODE, GL_LINEAR);
        glFogfv(GL_FOG_COLOR, fogcolor);
        glFogf(GL_FOG_DENSITY, 0.01);
        glFogf(GL_FOG_START, 35.0);
        glFogf(GL_FOG_END, 70.0);
    }
    else
    {
        glDisable(GL_FOG);
    }

    // Undo previous transformations
    glLoadIdentity();

    // Set camera position
    if (game)
    {
        double Ex = cx + 1.75*Sin(-cannon_turn);
        double Ey = cy + 0.75;
        double Ez = cz - 1.75*Cos(cannon_turn);
        gluLookAt(Ex,Ey,Ez, 0,Ey+Sin(-cannon_elev/1.5),0, 0,1,0);
    }
    else
    {
        double Ex = -dim*Sin(th)*Cos(ph);
        double Ey = +dim        *Sin(ph);
        double Ez = +dim*Cos(th)*Cos(ph);
        gluLookAt(Ex,Ey,Ez , 0,0,0 , 0,Cos(ph),0); 
    }
    skybox(sky[0], dim*3.5);

    // Lighting
    if (lighting)
    {
        float Ambient[]  = {0.0*ambient, 0.0*ambient, 0.0, 1.0};
        float Diffuse[]  = {0.01*diffuse, 0.005*diffuse, 0.0*diffuse, 1.0};
        float Specular[] = {0.01*specular, 0.005*specular, 0.0*specular, 1.0};
        // float Position[] = {Cos(zh)*30,15,Sin(zh)*30,1};
        float Position[] = {0,5,13.5,1};
        // glColor3f(1,1,1);
        // ball(Position[0], Position[1], Position[2], 0.2);

        glEnable(GL_NORMALIZE);
        glEnable(GL_LIGHTING);
        glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
        glEnable(GL_COLOR_MATERIAL);
        if (fwl.active) // Firework lighting
        {
            glDisable(GL_LIGHTING);
            for (int i = 0; i < parts; i++) {
                glColor3f(particles[i].r, particles[i].g, particles[i].b);
                ball(particles[i].x,particles[i].y,particles[i].z,partscale);
            }
            glEnable(GL_LIGHTING);
            float fwlP[] = {fwl.x, fwl.y, fwl.z, 1.0};
            float fwlA[] = {fwintense*fwl.r/5,fwintense*fwl.g/5,fwintense*fwl.b/5,1.0};
            float fwlD[] = {fwintense*fwl.r, fwintense*fwl.g, fwintense*fwl.b, 1.0};
            float fwlS[] = {fwintense*fwl.r, fwintense*fwl.g, fwintense*fwl.b, 1.0};
            glEnable(GL_LIGHT1);
            glLightfv(GL_LIGHT1,GL_POSITION,fwlP);
            glLightfv(GL_LIGHT1,GL_AMBIENT ,fwlA);
            glLightfv(GL_LIGHT1,GL_DIFFUSE ,fwlD);
            glLightfv(GL_LIGHT1,GL_SPECULAR,fwlS);
        }
        else
        {
            glDisable(GL_LIGHT1);
        }
        glEnable(GL_LIGHT0);
        glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient);
        glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
        glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
        glLightfv(GL_LIGHT0,GL_POSITION,Position);
    }
    else
        glDisable(GL_LIGHTING);

    /* Objects */
    cannon(cx,cy,cz, cannon_turn,cannon_elev,texture[0]);
    if (fw.active)
    {
        rocket(fw.x,fw.y,fw.z, fw.ft, fw.fe);
    }
    // Cannon tower
    tower(0,0,0, cy, texture[1]);

    // Ferris wheel
    ferris(0,0,15, fturn);
    woodcube(0,0.3,10, 4,0.3,3, texture[3]);
    woodcube(+4.25,0.15,10, 0.25,0.15,3, texture[3]);
    woodcube(-4.25,0.15,10, 0.25,0.15,3, texture[3]);
    woodcube(0,0.3,18, 4.5,0.3,1, texture[3]);
    glPushMatrix();
    glTranslated(0,2.5,18);
    glRotated(-35,1,0,0);
    woodcube(0,0,0, 0.2,3.2,0.2, texture[1]);
    glPopMatrix();
    seat(5*Cos(fturn),5*Sin(fturn)+5,15, 1,1,1, texture[3],texture[1]);
    seat(5*Cos(fturn+60),5*Sin(fturn+60)+5,15, 1,1,1, texture[3],texture[1]);
    seat(5*Cos(fturn+120),5*Sin(fturn+120)+5,15, 1,1,1, texture[3],texture[1]);
    seat(5*Cos(fturn+180),5*Sin(fturn+180)+5,15, 1,1,1, texture[3],texture[1]);
    seat(5*Cos(fturn+240),5*Sin(fturn+240)+5,15, 1,1,1, texture[3],texture[1]);
    seat(5*Cos(fturn+300),5*Sin(fturn+300)+5,15, 1,1,1, texture[3],texture[1]);

    // Tents
    tent(12,0,-6, 4,7,4, 225, 1,0.4,0, 1,1,1,       texture[4],texture[5]); // Orange/white
    tent(10,0,15, 5,5,5, 129, 0,0,1, 0,0.5,1,       texture[4],texture[5]); // Blue/light-blue
    tent(-9,0,15, 3,7,3, 70, 1,0,1, 0.5,0.1,0.75,   texture[4],texture[5]); // Pink/purple
    tent(14,0,4, 3,4,5, 170, 0,0,0, 1,0,0.2,        texture[4],texture[5]); // Red/black
    tent(6,0,-9, 2,4,2, 260, 0.8,0.8,0, 0,0.8,0.8,  texture[4],texture[5]); // Yellow/light-blue
    tent(-14,0,2, 3.5,6,3.5, 15, 0,1,0, 0,0.5,0,         texture[4],texture[5]); // Green/dark-green

    // Kiosks
    kiosk(2,0,-10, 1,1,1, 0,texture[3],texture[6], 1,0,0);
    kiosk(-2,0,-9.5, 2,1.5,1, 15,texture[3],texture[6], 0,1,0.25);
    kiosk(-6.5,0,-7.5, 2,1.2,1.2, 30,texture[3],texture[6], 0.25,0,1);
    kiosk(-12,0,9, 1.75,1.2,1, 120,texture[3],texture[6], 1,0,1);

    // Trees
    for (int i = 0; i < num_trees; i++)
    {
        tree(Cos(i*tree_angle)*tree_distance,0,Sin(i*tree_angle)*tree_distance, trees[i].x,trees[i].y,trees[i].z, 60,
        texture[8], texture[7]);
    }
    tree(-12,0,-12, 1.5,1.5,1.5, 60, texture[8], texture[7]);
    tree(-16,0,-16, 1.2,1.6,1.3, 60, texture[8], texture[7]);
    tree(-19,0,-4, 1,1.3,1.1, 60, texture[8], texture[7]);
    tree(-25,0,-6, 1.2,1.5,1, 60, texture[8], texture[7]);
    tree(24,0,11, 1.2,1.0,1.1, 60, texture[8], texture[7]);
    tree(25,0,2, 1.4,1.7,1.1, 60, texture[8], texture[7]);
    tree(17,0,-13, 1.5,2,1.5, 60, texture[8], texture[7]);
    tree(9,0,-22, 1.3,1.5,1.3, 60, texture[8], texture[7]);
    tree(-2,0,-18, 2,2.4,2, 60, texture[8], texture[7]);
    tree(-21,0,4, 1.8,1.7,1.8, 60, texture[8], texture[7]);
    tree(-18,0,12, 1.2,1.4,1.3, 60, texture[8], texture[7]);

    // Ground
    const int ground = 50;
    for (int i = -ground; i < ground; i++)
    {
        for (int j = -ground; j < ground; j++)
        {
            glColor3f(1,1,1);
            groundtile(i,0,j,1,texture[2]);
        }
    }

    // Axes
    glDisable(GL_LIGHTING);
    glColor3f(1,1,1);
    if (axes)
    {
        const double len=1.0; //  Length of axes
        glBegin(GL_LINES);
        glVertex3d(0.0,0.0,0.0);
        glVertex3d(len,0.0,0.0);
        glVertex3d(0.0,0.0,0.0);
        glVertex3d(0.0,len,0.0);
        glVertex3d(0.0,0.0,0.0);
        glVertex3d(0.0,0.0,len);
        glEnd();
        //  Label axes
        glRasterPos3d(len,0.0,0.0);
        Print("X");
        glRasterPos3d(0.0,len,0.0);
        Print("Y");
        glRasterPos3d(0.0,0.0,len);
        Print("Z");
    }

    // glWindowPos2i(5,5);
    // Print("Angle=%d,%d  Dim=%.1f",th,ph,dim);
    // glWindowPos2i(5,45);
    // Print("Firework at: %.2f, %.2f, %.2f | Speed: %.2f, %.2f, %.2f", fw.x, fw.y, fw.z, fw.dx, fw.dy, fw.dz);
    glColor3f(fw.r,fw.g,fw.b);
    glWindowPos2i(5,25);
    Print("Color: (%.2f, %.2f, %.2f)", fw.r,fw.g,fw.b);
    glColor3f(1,1,1);
    glWindowPos2i(5,5);
    Print("Cannon Turn = %d | Cannon Elevation = %d", cannon_turn, -cannon_elev);

    // Render scene and make it visible
    ErrCheck("display");
    glFlush();
    glutSwapBuffers();
}

/*
 *  GLUT calls this routine when idle
 */
void idle()
{
    //  Elapsed time in seconds
    double t = glutGet(GLUT_ELAPSED_TIME);
    if (fw.active)
    {
        if (t >= fw.time + flighttime)
        {
            fw.active = 0;
            if (!fwl.active) // Activate explosion
            {
                fwl.active = 1;
                fwl.time = t;
                fwl.x = fw.x;
                fwl.y = fw.y;
                fwl.z = fw.z;
                for (int i = 0; i < parts; i++)
                {
                    double xr = ((double)rand() / (double)RAND_MAX * 2 - 1);
                    double yr = ((double)rand() / (double)RAND_MAX * 2 - 1);
                    double zr = ((double)rand() / (double)RAND_MAX * 2 - 1);
                    double rr = ((double)rand() / (double)RAND_MAX / 7);
                    double gr = ((double)rand() / (double)RAND_MAX / 7);
                    double br = ((double)rand() / (double)RAND_MAX / 7);
                    particles[i].x = fwl.x; particles[i].dx = xr/125;
                    particles[i].y = fwl.y; particles[i].dy = yr/125;
                    particles[i].z = fwl.z; particles[i].dz = zr/125;
                    particles[i].r = fwl.r + rr;
                    particles[i].g = fwl.g + gr;
                    particles[i].b = fwl.b + br;
                    partscale = 0.1;
                }
            }
        }
        else
        {
            fw.x += (fw.dx / speed);
            fw.y += (fw.dy / speed);
            fw.z += (fw.dz / speed);
        }
    }
    if (fwl.active)
    {
        partscale -= 0.0004;
        for (int i = 0; i < parts; i++) // update particle position
        {
            particles[i].x = particles[i].x + particles[i].dx;
            particles[i].y = particles[i].y + particles[i].dy;
            particles[i].dy = particles[i].dy - 0.00005;
            particles[i].z = particles[i].z + particles[i].dz;
        }
        if (t >= fwl.time + 2000)
        {
            fwl.active = 0;
        }
        fwintense = 1 - ((t - fwl.time) / 2000.0);
    }
    zh = fmod(45*(t/1000.0),360.0);
    fturn += 0.1;
    if (fturn >= 360)
        fturn -= 360;
    //  Tell GLUT it is necessary to redisplay the scene
    glutPostRedisplay();
}

/*
 *  GLUT calls this routine when an arrow key is pressed
 */
void special(int key,int x,int y)
{
    if (!game) // Spectator only controls
    {
        //  Right arrow key - increase angle by 5 degrees
        if (key == GLUT_KEY_RIGHT)
            th += 5;
        //  Left arrow key - decrease angle by 5 degrees
        else if (key == GLUT_KEY_LEFT)
            th -= 5;
        //  Up arrow key - increase elevation by 5 degrees
        else if (key == GLUT_KEY_UP)
            ph += 5;
        //  Down arrow key - decrease elevation by 5 degrees
        else if (key == GLUT_KEY_DOWN)
            ph -= 5;
        //  PageUp key - increase dim
        else if (key == GLUT_KEY_PAGE_DOWN)
            dim += 0.1;
        //  PageDown key - decrease dim
        else if (key == GLUT_KEY_PAGE_UP && dim>1)
            dim -= 0.1;
    }
    //  Keep angles to +/-360 degrees
    th %= 360;
    ph %= 360;
    //  Update projection
    Project(55,asp,dim);
    //  Tell GLUT it is necessary to redisplay the scene
    glutPostRedisplay();
}

/*
 *  GLUT calls this routine when a key is pressed
 */
void key(unsigned char ch,int x,int y)
{
    // Exit on ESC
    if (ch == 27)
        exit(0);
    // Reset view angle
    else if (ch == '0')
    {
        if (game)
            cannon_turn = cannon_elev = 0;
        else
            th = ph = 0;
    }
    // Game keys
    else if (ch == 'p' || ch == 'P') // Toggle gamemode
        game = 1 - game;
    else if (ch == 'a' || ch == 'A')
        cannon_turn += 2;
    else if (ch == 'd' || ch == 'D')
        cannon_turn -= 2;
    else if (ch == 'w' || ch == 'W')
    {
        if (cannon_elev > -70)
            cannon_elev -= 2;
    }
    else if (ch == 's' || ch == 'S')
    {
        if (cannon_elev < 20)
            cannon_elev += 2;
    }
    else if (ch == ' ') // Spacebar shoots a firework
    {
        if (!fw.active) // Basically creates a firework with the right vector and position
        {
            fw.active = 1;
            fw.time = glutGet(GLUT_ELAPSED_TIME);
            double since = Sin(-cannon_elev);
            double sinct = Sin(cannon_turn);
            double cosct = Cos(cannon_turn);
            fw.x = cx + (sinct * Cos(cannon_elev));
            fw.y = cy + since + 0.25;
            fw.z = cz + (cosct * Cos(cannon_elev));
            double dx = sinct * Cos(cannon_elev);
            double dy = since;
            double dz = cosct * Cos(cannon_elev);
            // Normalize
            double len = sqrt((dx*dx)+(dy*dy)+(dz*dz));
            if (len > 0)
            {
                dx /= len;
                dy /= len;
                dz /= len;
            }
            fw.dx = dx;
            fw.dy = dy;
            fw.dz = dz;
            fw.ft = cannon_turn;
            fw.fe = cannon_elev;
            fwl.r = fw.r; // Store current color in the light (can't be changed after launch)
            fwl.g = fw.g;
            fwl.b = fw.b;
        }
    }
    else if (ch == 'r' && fw.r >= colorchange)
        fw.r -= colorchange;
    else if (ch == 'R' && fw.r <= 1-colorchange)
        fw.r += colorchange;
    else if (ch == 'g' && fw.g >= colorchange)
        fw.g -= colorchange;
    else if (ch == 'G' && fw.g <= 1-colorchange)
        fw.g += colorchange;
    else if (ch == 'b' && fw.b >= colorchange)
        fw.b -= colorchange;
    else if (ch == 'B' && fw.b <= 1-colorchange)
        fw.b += colorchange;
    else if (ch == 'f' || ch == 'F')
        fog = 1 - fog;

    cannon_turn %= 360;
    cannon_elev %= 360;

    //  Reproject
    Project(55,asp,dim);
    //  Tell GLUT it is necessary to redisplay the scene
    glutPostRedisplay();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void reshape(int width,int height)
{
   //  Ratio of the width to the height of the window
   asp = (height>0) ? (double)width/height : 1;
   //  Set the viewport to the entire window
   glViewport(0,0, RES*width,RES*height);
   //  Set projection
   Project(55,asp,dim);
}

int main(int argc, char* argv[])
{
    // Initialize GLUT
    glutInit(&argc,argv);
    //  Request double buffered, true color window with Z buffering at 600x600
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(600,600);
    glutCreateWindow("Pierce Angeloff - Final");
#ifdef USEGLEW
    //  Initialize GLEW
    if (glewInit()!=GLEW_OK) Fatal("Error initializing GLEW\n");
#endif
    //  Set callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(special);
    glutKeyboardFunc(key);
    glutIdleFunc(idle);
    // Load textures
    texture[0] = LoadTexBMP("cannon.bmp");
    texture[1] = LoadTexBMP("metal.bmp");
    texture[2] = LoadTexBMP("grass.bmp");
    texture[3] = LoadTexBMP("wood.bmp");
    texture[4] = LoadTexBMP("tentopen.bmp");
    texture[5] = LoadTexBMP("ripples.bmp");
    texture[6] = LoadTexBMP("fabric.bmp");
    texture[7] = LoadTexBMP("leaves.bmp");
    texture[8] = LoadTexBMP("bark.bmp");
    sky[0] = LoadTexBMP("stars.bmp");
    // Calculate Cannon norms
    CalcNorm();
    // Generate trees
    generateTrees();
    //  Pass control to GLUT so it can interact with the user
    ErrCheck("init");
    glutMainLoop();
}