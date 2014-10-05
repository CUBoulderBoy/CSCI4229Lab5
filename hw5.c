/*
 * Lab 5: Snowfall with Projections and Lighting
 * Christopher Jordan
 * CSCI 4229 Fall 2014
 *
 * Utilizing code provided from William Schreuder
 * via the ex9 and ex13 programs
 *
 *  Key bindings:
 *  l          Toggles lighting
 *  a/A        Decrease/increase ambient light
 *  d/D        Decrease/increase diffuse light
 *  s/S        Decrease/increase specular light
 *  e/E        Decrease/increase emitted light
 *  n/N        Decrease/increase shininess
 *  F1         Toggle smooth/flat shading
 *  F2         Toggle local viewer mode
 *  F3         Toggle light distance (1/5)
 *  F8         Change ball increment
 *  F9         Invert bottom normal
 *  m          Toggles light movement
 *  []         Lower/rise light
 *  p          Toggles ortogonal/perspective projection
 *  +/-        Change field of view of perspective
 *  x          Toggle axes
 *  arrows     Change view angle
 *  PgDn/PgUp  Zoom in and out
 *  0          Reset view angle
 *  ESC        Exit
 */
#include "CSCIx229.h"
#include <time.h>

int axes=0;       //  Display axes
int mode=1;       //  Projection mode
int move=1;       //  Move light
int th=0;         //  Azimuth of view angle
int ph=0;         //  Elevation of view angle
int fov=55;       //  Field of view (for perspective)
int light=1;      //  Lighting
double asp=1;     //  Aspect ratio
double dim=15.0;   //  Size of world

// Light values
int one       =   1;  // Unit value
int distance  =  15;  // Light distance
int inc       =  10;  // Ball increment
int smooth    =   1;  // Smooth/Flat shading
int local     =   0;  // Local Viewer Model
int emission  =   0;  // Emission intensity (%)
int ambient   =  30;  // Ambient intensity (%)
int diffuse   = 100;  // Diffuse intensity (%)
int specular  =   0;  // Specular intensity (%)
int shininess =   0;  // Shininess (power of two)
float shinyvec[1];    // Shininess (value)
int zh        =  90;  // Light azimuth
float ylight  =   0;  // Elevation of light

// Snow fall variables
int new_snow=1;   //  Global to determine if arrays need to be populated
int frozen=1;     //  Frozen or falling snow
int fall_down=1;

// For idle function when snow falling
unsigned long ot;
unsigned long dt;

// For initclock
int first=0;

// Arrays to store snowflake data
#define NUM_SNOWFLAKES 20
double x_ar[NUM_SNOWFLAKES];
double y_ar[NUM_SNOWFLAKES];
double z_ar[NUM_SNOWFLAKES];
double s_ar[NUM_SNOWFLAKES];
double m_ar[NUM_SNOWFLAKES];

//  Macro for sin & cos in degrees
double Ex, Ey, Ez;
double Vx = 0, Vy = 0, Vz =  0;
int strafe = 0;

// Mouse variables
int xOrigin = -1;
int yOrigin = -1;
int m_button = 0;

/*
 *  Draw an ice crystal 
 *     
 */
static void ice_crystal(double x,double y,double z,
                 double dx,double dy,double dz,
                 double rx, double ry, double rz,
                 double th)
{
   //  Save transformation
   glPushMatrix();
   
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,rx,ry,rz);
   glScaled(dx,dy,dz);
   
   //  Crystal
   glBegin(GL_TRIANGLES);

   //  Front Top
   glColor3f(0.0,0.0,1.0);
   glNormal3f(+0,+2,+10);
   glVertex3f(+0,+5,+0);
   glVertex3f(-1,+0,+1);
   glVertex3f(+1,+0,+1);

   //  Right Top
   glNormal3f(+10,+2,+0);
   glVertex3f(+0,+5,+0);
   glVertex3f(+1,+0,+1);
   glVertex3f(+1,+0,-1);

   //  Back Top
   glNormal3f(+0,+2,-10);
   glVertex3f(+0,+5,+0);
   glVertex3f(+1,+0,-1);
   glVertex3f(-1,+0,-1);

   //  Left Top
   glNormal3f(-10,+2,+0);
   glVertex3f(+0,+5,+0);
   glVertex3f(-1,+0,-1);
   glVertex3f(-1,+0,+1);

   //  Front Bottom
   glNormal3f(+0,-2,-10);
   glVertex3f(+0,-5,+0);
   glVertex3f(-1,+0,+1);
   glVertex3f(+1,+0,+1);

   //  Right Bottom
   glNormal3f(+10,-2,+0);
   glVertex3f(+0,-5,+0);
   glVertex3f(+1,+0,+1);
   glVertex3f(+1,+0,-1);

   //  Back Bottom
   glNormal3f(+0,-2,-10);
   glVertex3f(+0,-5,+0);
   glVertex3f(+1,+0,-1);
   glVertex3f(-1,+0,-1);

   //  Left Bottom
   glNormal3f(-10,-2,+0);
   glVertex3f(+0,-5,+0);
   glVertex3f(-1,+0,-1);
   glVertex3f(-1,+0,+1);

   //  End
   glEnd();
   //  Undo transofrmations
   glPopMatrix();
}

/*
 *  Draw a snowflake
 *     at (x,y,z)
 *     dimentions (crystal(0,0,0, 1,1,1 , 0);dx,dy,dz)
 *     rotated th about the y axis
 */
 static void snowflake(double x,double y,double z,
   double dx, double dy, double dz, double m, double r)
 {
   ice_crystal(x,y,z, dx,dy,dz, r,r+1,r, 0);
   ice_crystal(x,y,z, dx/m,dy/m,dz/m, r+1,r,r+1, 45);
   ice_crystal(x,y,z, dx/m,dy/m,dz/m, r-1,r,r-1, 45);
   ice_crystal(x,y,z, dx/m,dy/m,dz/m, r+1,r,r-1, 45);
   ice_crystal(x,y,z, dx/m,dy/m,dz/m, r-1,r,r+1, 45);
}

/*
 *  Draw vertex in polar coordinates with normal
 */
static void Vertex(double th,double ph)
{
   double x = Sin(th)*Cos(ph);
   double y = Cos(th)*Cos(ph);
   double z =         Sin(ph);
   //  For a sphere at the origin, the position
   //  and normal vectors are the same
   glNormal3d(x,y,z);
   glVertex3d(x,y,z);
}

/*
 *  Draw a ball
 *     at (x,y,z)
 *     radius (r)
 */
static void ball(double x,double y,double z,double r)
{
   int th,ph;
   float yellow[] = {1.0,1.0,0.0,1.0};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   //  Save transformation
   glPushMatrix();
   //  Offset, scale and rotate
   glTranslated(x,y,z);
   glScaled(r,r,r);
   //  White ball
   glColor3f(1,1,1);
   glMaterialfv(GL_FRONT,GL_SHININESS,shinyvec);
   glMaterialfv(GL_FRONT,GL_SPECULAR,yellow);
   glMaterialfv(GL_FRONT,GL_EMISSION,Emission);
   //  Bands of latitude
   for (ph=-90;ph<90;ph+=inc)
   {
      glBegin(GL_QUAD_STRIP);
      for (th=0;th<=360;th+=2*inc)
      {
         Vertex(th,ph);
         Vertex(th,ph+inc);
      }
      glEnd();
   }
   //  Undo transofrmations
   glPopMatrix();
}

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
   // Temporary variables for snowflake data calculation
   int i,x,y,z;
   double s,m;
   int int_dim = (int)dim;

   const double len=2.0;  //  Length of axes
   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);

   //  Undo previous transformations
   glLoadIdentity();
   //  Perspective - set eye position
   if (mode)
   {
      double Ex2 = -2*dim*Sin(th)*Cos(ph);
      double Ey2 = +2*dim        *Sin(ph);
      double Ez2 = +2*dim*Cos(th)*Cos(ph);
      if ( strafe ){
         // Change vision point based on change in eye
         Vx += Ex2 - Ex;
         Vx += Ex2 - Ex;
         Vx += Ex2 - Ex;
      }

      // Set eye variables
      Ex = Ex2;
      Ey = Ey2;
      Ez = Ez2;

      // Call gluLookAt
      gluLookAt(Ex,Ey,Ez, Vx,Vy,Vz , 0,Cos(ph),0);
   }

   //  Orthogonal - set world orientation
   else
   {
      glRotatef(ph,1,0,0);
      glRotatef(th,0,1,0);
   }

   //  Flat or smooth shading
   glShadeModel(smooth ? GL_SMOOTH : GL_FLAT);

   //  Light switch
   if (light)
   {
        //  Translate intensity to color vectors
        float Ambient[]   = {0.01*ambient ,0.01*ambient ,0.01*ambient ,1.0};
        float Diffuse[]   = {0.01*diffuse ,0.01*diffuse ,0.01*diffuse ,1.0};
        float Specular[]  = {0.01*specular,0.01*specular,0.01*specular,1.0};
        //  Light position
        float Position[]  = {distance*Cos(zh),ylight,distance*Sin(zh),1.0};
        //  Draw light position as ball (still no lighting here)
        glColor3f(1,1,1);
        ball(Position[0],Position[1],Position[2] , 0.1);
        //  OpenGL should normalize normal vectors
        glEnable(GL_NORMALIZE);
        //  Enable lighting
        glEnable(GL_LIGHTING);
        //  Location of viewer for specular calculations
        glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,local);
        //  glColor sets ambient and diffuse color materials
        glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
        glEnable(GL_COLOR_MATERIAL);
        //  Enable light 0
        glEnable(GL_LIGHT0);
        //  Set ambient, diffuse, specular components and position of light 0
        glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient);
        glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
        glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
        glLightfv(GL_LIGHT0,GL_POSITION,Position);
   }
   else
     glDisable(GL_LIGHTING);

   // Snowfall frozen in time
   if( new_snow == 1 ){
      for (i = 0; i < NUM_SNOWFLAKES; i++){
         // Set x for i in array
         x = rand() % int_dim;
         if (rand() % 2 == 0){
            x = x * -1;
         }
         x_ar[i] = x;

         // Set y for i in array
         y = rand() % int_dim;
         if (rand() % 2 == 0){
            y = y * -1;
         }
         y_ar[i] = y;

         // Set z for i in array
         z = rand() % int_dim;
         if (rand() % 2 == 0){
            z = z * -1;
         }
         z_ar[i] = z;

         // Set size of primary ice crystal
         s = rand() % int_dim;
         if (s == 0.0){
            s = 0.5;
         }
         else{
            s = 1/s;
         }
         s_ar[i] = s;

         // Set size modifier for secondary ice crystals
         m = rand() % int_dim;
         if (m == 0.0){
            m = 1.3;
         }
         else{
            m = 1 + 1/m;
         }
         m_ar[i] = m;
      }  
      new_snow = 0;
   }  

   if( frozen == 1 || fall_down == 0){
      // Building snowfall
      for (i = 0; i < NUM_SNOWFLAKES; i++){
         snowflake(x_ar[i], y_ar[i], z_ar[i], s_ar[i],s_ar[i],s_ar[i], m_ar[i], 0);
      }
   }
   else {
      for (i = 0; i < NUM_SNOWFLAKES; i++){
         // Create snowflake
         snowflake(x_ar[i], y_ar[i], z_ar[i], s_ar[i],s_ar[i],s_ar[i], m_ar[i], 0);

         // Move snowflake down
         y_ar[i] = y_ar[i] - 0.4;

         // If snowflake has fallen off view, move back to top
         if (y_ar[i] < -dim){
            y_ar[i] = dim;
         }
      }
      fall_down = 0;
   }

   //  Draw axes - no lighting from here on
   glDisable(GL_LIGHTING);
   glColor3f(1,1,1);
   if (axes)
   {
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

   //  Display parameters
   glWindowPos2i(5,5);
   Print("p/P: View Mode | Mouse LButton: Rotate Camera | Mouse RButton: Fwd/Bkwd and R/L Strafe");

   //  Display parameters
   glWindowPos2i(5,25);
   Print("f/F: Frozen/Falling Snow | PgUp: Move Fwd | PdDn: Move Bkwd | Arrows: Rotate Camera");
   
   //  Display parameters
   glWindowPos2i(5,45);
   Print("Angle=%d,%d  Dim=%.1f FOV=%d Projection=%s Light=%s",
     th,ph,dim,fov,mode?"Perpective":"Orthogonal",light?"On":"Off");

   // If light is on display lighting
   if (light)
   {
      glWindowPos2i(5,65);
      Print("Model=%s LocalViewer=%s Distance=%d Elevation=%.1f",smooth?"Smooth":"Flat",local?"On":"Off",distance,ylight);
      glWindowPos2i(5,85);
      Print("Ambient=%d  Diffuse=%d Specular=%d Emission=%d Shininess=%.0f",ambient,diffuse,specular,emission,shinyvec[0]);
   }

   //  Render the scene and make it visible
   ErrCheck("display");
   glFlush();
   glutSwapBuffers();
}

/*
 *  Idle function for falling snow
 */
void idle()
{
   double rho;

   //  Elapsed time in seconds
   double t = glutGet(GLUT_ELAPSED_TIME)/1000.0;
   zh = fmod(90*t,360.0);

   // For snowfall
   dt = clock();
   rho = (double)(dt - ot)/CLOCKS_PER_SEC;
   if (rho >= 0.1){
      ot = dt;
      if(frozen == 0){
         fall_down = 1;
      }
   }

   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when an arrow key is pressed
 */
void special(int key,int x,int y)
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
      dim -= 0.1;
   
   //  PageDown key - decrease dim
   else if (key == GLUT_KEY_PAGE_UP && dim>1)
      dim += 0.1;
   
   //  Smooth color model
   else if (key == GLUT_KEY_F1)
      smooth = 1-smooth;
   
   //  Local Viewer
   else if (key == GLUT_KEY_F2)
      local = 1-local;
   else if (key == GLUT_KEY_F3)
      distance = (distance==1) ? 5 : 1;
   
   //  Toggle ball increment
   else if (key == GLUT_KEY_F8)
      inc = (inc==10)?3:10;
   
   //  Flip sign
   else if (key == GLUT_KEY_F9)
      one = -one;
   
   //  Keep angles to +/-360 degrees
   th %= 360;
   ph %= 360;
   
   //  Update projection
   Project(mode?fov:0,asp,dim);
   
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when a key is pressed
 */
void key(unsigned char ch,int x,int y)
{
   //  Exit on ESC
   if (ch == 27)
      exit(0);
   //  Reset view angle
   else if (ch == '0')
      th = ph = 0;
   //  Toggle axes
   else if (ch == 'x' || ch == 'X')
      axes = 1-axes;
   //  Toggle lighting
   else if (ch == 'l' || ch == 'L')
      light = 1-light;
   //  Switch projection mode
   else if (ch == 'p' || ch == 'P')
      mode = 1-mode;
   //  Toggle light movement
   else if (ch == 'm' || ch == 'M')
      move = 1-move;
   
   //  Toggle snow falling vs frozen
   else if (ch == 'f' || ch == 'F')
      frozen = 1-frozen;
   //  Force new snow in frozen view
   else if (ch == 'n' || ch == 'N')
      new_snow = 1;

   //  Move light
   else if (ch == '<')
      zh += 1;
   else if (ch == '>')
      zh -= 1;
   //  Change field of view angle
   else if (ch == '-' && ch>1)
      fov--;
   else if (ch == '+' && ch<179)
      fov++;
   //  Light elevation
   else if (ch=='[')
      ylight -= 0.1;
   else if (ch==']')
      ylight += 0.1;
   //  Ambient level
   else if (ch=='a' && ambient>0)
      ambient -= 5;
   else if (ch=='A' && ambient<100)
      ambient += 5;
   //  Diffuse level
   else if (ch=='d' && diffuse>0)
      diffuse -= 5;
   else if (ch=='D' && diffuse<100)
      diffuse += 5;
   //  Specular level
   else if (ch=='s' && specular>0)
      specular -= 5;
   else if (ch=='S' && specular<100)
      specular += 5;
   //  Emission level
   else if (ch=='e' && emission>0)
      emission -= 5;
   else if (ch=='E' && emission<100)
      emission += 5;
   //  Shininess level
   else if (ch=='n' && shininess>-1)
      shininess -= 1;
   else if (ch=='N' && shininess<7)
      shininess += 1;
   //  Translate shininess power to value (-1 => 0)
   shinyvec[0] = shininess<0 ? 0 : pow(2.0,shininess);
   //  Reproject
   Project(mode?fov:0,asp,dim);
   //  Animate if requested
   glutIdleFunc(move?idle:NULL);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when the mouse button pressed
 */
void mouseButton (int button, int state, int x, int y) {
   xOrigin = x; 
   yOrigin = y;
   if( button == GLUT_LEFT_BUTTON){
      m_button = 0;
   }
   if( button == GLUT_RIGHT_BUTTON){
      m_button = 1;
   }
}

/*
 *  GLUT calls this routine when the mouse moves
 */
void mouseMove(int x, int y) {
   if ( m_button == 0){
      //  Sideways angle view
      strafe = 0;
      th += (xOrigin - x)/2;
      th %= 360;
      xOrigin = x;

      // Up and down angle view
      ph += (y - yOrigin)/2;
      ph %= 360;
      yOrigin = y;
   }
   if ( m_button == 1){
      //  Right and left movement changes
      strafe = 1;
      th += (xOrigin - x)/2;
      th %= 360;
      xOrigin = x;

      // Forward and backward movement
      if ( mode ){
         fov += (y - yOrigin)/2;
      }
      else{
         dim += (y - yOrigin)/2;
      }
      yOrigin = y;
   }
   //  Reproject
   Project(mode?fov:0,asp,dim);

   //  Redisplay the scene
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
   glViewport(0,0, width,height);
   //  Set projection
   Project(mode?fov:0,asp,dim);
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{
   //  Initialize GLUT
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   glutInitWindowSize(800,800);
   glutCreateWindow("Christopher Jordan Lab5: Snow with Lighting");
   //  Set callbacks
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   
   // Set keyboard functions
   glutSpecialFunc(special);
   glutKeyboardFunc(key);

   // Set idle functions
   glutIdleFunc(idle);

   //  Mouse handler functions
   glutMouseFunc(mouseButton);
   glutMotionFunc(mouseMove);

   // Set clock
   ot = clock();

   //  Pass control to GLUT so it can interact with the user
   ErrCheck("init");
   glutMainLoop();
   return 0;
}
