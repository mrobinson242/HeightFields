/*                              */
/*  CSCI 420 Computer Graphics  */
/*  Assignment 1: Height Fields */
/*  Author: Matt Robinson       */
/*  Student Id: 9801107811      */
/*                              */

// Headers
#include <stdlib.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <pic.h>
#include <iostream>
#include <sstream>
#include <math.h>

// Member Variables
int g_iMenuId;
int g_vMousePos[2] = {0, 0};
int g_iLeftMouseButton = 0;    /* 1 if pressed, 0 if not */
int g_iMiddleMouseButton = 0;
int g_iRightMouseButton = 0;

// Transform Enum
typedef enum { ROTATE, TRANSLATE, SCALE } CONTROLSTATE;
CONTROLSTATE g_ControlState = ROTATE;

// Point Object
typedef float point3[4];

// Translate Enabled Indicator
bool _translateEnabled;

// Lights On Indicators
bool _light0Enabled;
bool _light1Enabled;
bool _light2Enabled;

// State of the World Transformations */
float g_vLandRotate[3] = {30.0, 10.0, -10.0};
float g_vLandTranslate[3] = {0, 0, 10.0};
float g_vLandScale[3] = {0.005, 0.005, 0.005};

// Saved File Counter
int _count = 0;

// Pic Object (see <your pic directory>/pic.h for type Pic)
Pic *pic;

/**
 * saveScreenshot - Write a screenshot to the
 *                  specified filename
 */
void saveScreenshot (char *filename)
{
  int i, j;
  Pic *in = NULL;

  if (filename == NULL)
    return;

  /* Allocate a picture buffer */
  in = pic_alloc(640, 480, 3, NULL);

  printf("File to save to: %s\n", filename);

  for (i=479; i>=0; i--) {
    glReadPixels(0, 479-i, 640, 1, GL_RGB, GL_UNSIGNED_BYTE,
                 &in->pix[i*in->nx*in->bpp]);
  }

  if (jpeg_write(filename, in))
  {
    // Log Successful File Saving
    printf("File saved Successfully\n");

    // Increment File Count
    _count++;
  }
  else
  {
    // Log Error in File Saving
    printf("Error in Saving\n"); 
  }

  pic_free(in);
}

/*
 * init
 */ 
void init()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);   // set background color
    glEnable(GL_DEPTH_TEST);            // enable depth buffering
    glShadeModel(GL_SMOOTH);            // interpolate colors during rasterization
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Initialize Translate Transformation
    g_vLandTranslate[0] = -pic->nx/2;
    g_vLandTranslate[1] = -pic->ny/2;

    // Initialize Keyboard Indicators
    _light0Enabled = false;
    _light1Enabled = false;
    _light2Enabled = false;
    _translateEnabled = false;

    // Initialize Lighting 0 Components (Blue)
    GLfloat lightDiffuse0[] = {0.0, 0.0, 1.0, 1.0};
    GLfloat lightSpecular0[] = {0.0, 0.0, 1.0, 1.0};
    GLfloat lightAmbient0[] = {0.0, 0.0, 1.0, 1.0};
    GLfloat lightPosition0[] = {50.0, 0.0, 0.0, 0.0};

    // Set up Components for Light 0
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse0);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular0);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition0);

    // Initialize Lighting 1 Components (Red)
    GLfloat lightDiffuse1[] = {1.0, 0.0, 0.0, 1.0};
    GLfloat lightSpecular1[] = {1.0, 0.0, 0.0, 1.0};
    GLfloat lightAmbient1[] = {1.0, 0.0, 0.0, 1.0};
    GLfloat lightPosition1[] = {50.0, 0.0, 0.0, 0.0};

    // Set up Components for Light 1
    glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmbient1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse1);
    glLightfv(GL_LIGHT1, GL_SPECULAR, lightSpecular1);
    glLightfv(GL_LIGHT1, GL_POSITION, lightPosition1);

    // Initialize Lighting 2 Components (Green)
    GLfloat lightDiffuse2[] = {0.0, 1.0, 0.0, 1.0};
    GLfloat lightSpecular2[] = {0.0, 1.0, 0.0, 1.0};
    GLfloat lightAmbient2[] = {0.0, 1.0, 0.0, 1.0};
    GLfloat lightPosition2[] = {50.0, 0.0, 0.0, 0.0};

    // Set up Components for Light 2
    glLightfv(GL_LIGHT2, GL_AMBIENT, lightAmbient2);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, lightDiffuse2);
    glLightfv(GL_LIGHT2, GL_SPECULAR, lightSpecular2);
    glLightfv(GL_LIGHT2, GL_POSITION, lightPosition2);

    // Initialize Material Properites
    float shine = 100.0;
    glMaterialf(GL_FRONT, GL_SHININESS, shine);
}

/**
 * display
 */
void display()
{
    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity(); // reset transformation

    // Scale
    glScalef(g_vLandScale[0], g_vLandScale[1], g_vLandScale[2]);

    // Rotate
    glRotatef(g_vLandRotate[0], 1, 0, 0);
    glRotatef(g_vLandRotate[1], 0, 1, 0);
    glRotatef(g_vLandRotate[2], 0, 0, 1);

    // Translate (Back to Center of Image)
    glTranslatef(g_vLandTranslate[0], g_vLandTranslate[1], g_vLandTranslate[2]);

    // Iterate over image Y Values
    for(int y=0; y<pic->ny-1; y++)
    {
        // Initialize Tri-Strip Creation
        glBegin(GL_TRIANGLE_STRIP);
   
        // Iterate over image x Values
        for(int x=0; x<pic->nx; x++)
        {
            // Get Height Values for initial 2 vertices
            unsigned char z1 = PIC_PIXEL(pic, x, y, 0);   // Top Vertex
            unsigned char z2 = PIC_PIXEL(pic, x, y+1, 0); // Bottom Vertex

            // Modify Height Values
            float z1f = (z1/255.0f);
            float z2f = (-40 * (z2/255.0f));

            // Specify Top Vertex
            glColor3f(z1/255.0f, z1/255.0f, 1.0);
            point3 a = {x, y, z1f};
            glNormal3f(x,y,0);
            glVertex3f(x, y, z1f);

            // Specify Bottom Vertex
            glColor3f(z2/255.0f, z2/255.0f, 1.0);
            point3 b = {x, y+1, z2f};
            glNormal3f(x,y+1,0);
            glVertex3f(x, y+1, z2f);
        }
        // End Tri-Strip Creation
        glEnd();
    }

    // double buffer flush
    glutSwapBuffers();
}

/**
 * menufunc - Handles Callbacks to Selections in Right Click Menu
 */
void menufunc(int value)
{
  switch (value)
  {
    case 0:
      exit(0);
      break;

    case 1:
      // Initialize StringStream
      std::stringstream ss;

      // Get Saved Image Count
      ss << _count;
      std::string s = ss.str();

      // Clear StringStream
      ss.str(std::string());

      // Check if Length of String
      if(s.length() == 1)
      {
          ss << "00";
          ss << s;
      }
      else if(s.length() == 2)
      {
          ss << "0";
          ss << s;
      }
      else
      {
          ss << s;
      }

      // Add File Extension
      ss << ".jpg";

      // Create File String
      std::string fileString = ss.str();
      char* fileName = (char*) fileString.c_str();

      // Save Screenshot
      saveScreenshot(fileName);
      break;
  }
}

/**
 * doIdle
 */
void doIdle()
{
  // Make the screen update
  glutPostRedisplay();
}

/**
 * mouseDrag - Converts mouse drags into information
 *             about rotation/translation/scaling
 */
void mousedrag(int x, int y)
{
  int vMouseDelta[2] = {x-g_vMousePos[0], y-g_vMousePos[1]};
  
  switch (g_ControlState)
  {
    case TRANSLATE:  
      if (g_iLeftMouseButton)
      {
        g_vLandTranslate[0] += vMouseDelta[0];
        g_vLandTranslate[1] -= vMouseDelta[1];
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandTranslate[2] += vMouseDelta[1];
      }
      break;
    case ROTATE:
      if (g_iLeftMouseButton)
      {
        g_vLandRotate[0] += vMouseDelta[1];
        g_vLandRotate[1] += vMouseDelta[0];
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandRotate[2] += vMouseDelta[1];
      }
      break;
    case SCALE:
      if (g_iLeftMouseButton)
      {
        g_vLandScale[0] *= 1.0+vMouseDelta[0]*0.01;
        g_vLandScale[1] *= 1.0-vMouseDelta[1]*0.01;
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandScale[2] *= 1.0-vMouseDelta[1]*0.01;
      }
      break;
  }
  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

/*
 * reshape - Called every time window is resized
 *           to update the projection matrix, and
 *           to preserve aspect ratio
 */
void reshape(int w, int h)
{
    // Initialize Aspect
    GLfloat aspect = (GLfloat)w/(GLfloat)h;

    // Setup image size
    glViewport(0, 0, w, h);

    // Set Perspective
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(0.0, aspect, 0.01, 1000.0);

    // Set back to ModelView
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/**
 * mouseidle
 */
void mouseidle(int x, int y)
{
  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

/**
 * mousebutton
 */
void mousebutton(int button, int state, int x, int y)
{
  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      g_iLeftMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_MIDDLE_BUTTON:
      g_iMiddleMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_RIGHT_BUTTON:
      g_iRightMouseButton = (state==GLUT_DOWN);
      break;
  }
 
  switch(glutGetModifiers())
  {
    case GLUT_ACTIVE_CTRL:
      g_ControlState = TRANSLATE;
      break;

    case GLUT_ACTIVE_SHIFT:
      g_ControlState = SCALE;
      break;

    default:
      if(_translateEnabled)
      {
          g_ControlState = TRANSLATE;
      }
      else
      {
          g_ControlState = ROTATE;
      }
      break;
  }

  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

/** 
 * processKeys - Handles Callbacks for Keyboard Keys
 */
void processKeys(unsigned char key, int x, int y)
{
    switch(key)
    {
        // Translate Case
        case 't':
        case 'T':
            // Check if Translate already Enabled
            if(_translateEnabled)
            {
                // Disable Translate Mode
                _translateEnabled = false;
            }
            else
            {
                // Enable Translate Mode
                _translateEnabled = true;
            }

            // Update Mouse Position
            g_vMousePos[0] = x;
            g_vMousePos[1] = y;
            break;

        // Light 0
        case '0':
            if(_light0Enabled)
            {
               // Disable Light Mode
               _light0Enabled = false;
               glDisable(GL_LIGHT0);

               // Check whether to diable Lighting
               if(!_light0Enabled && !_light1Enabled && !_light2Enabled)
               {
                   glDisable(GL_LIGHTING);
               }
            }
            else
            {
                _light0Enabled = true;
                glEnable(GL_LIGHTING);
                glEnable(GL_LIGHT0);
            }
            break;

        // Light 1
        case '1':
            if(_light1Enabled)
            {
               // Disable Light Mode
               _light1Enabled = false;
               glDisable(GL_LIGHT1);

               // Check whether to diable Lighting
               if(!_light0Enabled && !_light1Enabled && !_light2Enabled)
               {
                   glDisable(GL_LIGHTING);
               }
            }
            else
            {
                _light1Enabled = true;
                glEnable(GL_LIGHTING);
                glEnable(GL_LIGHT1);
            }
            break;

        // Light 2
        case '2':
            if(_light2Enabled)
            {
               // Disable Light Mode
               _light2Enabled = false;
               glDisable(GL_LIGHT2);

               // Check whether to diable Lighting
               if(!_light0Enabled && !_light1Enabled && !_light2Enabled)
               {
                   glDisable(GL_LIGHTING);
               }
            }
            else
            {
                _light2Enabled = true;
                glEnable(GL_LIGHTING);
                glEnable(GL_LIGHT2);
            }
            break;

        // Points
        case 'p':
        case 'P':
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            break;

        // (Wireframe) Lines
        case 'l':
        case 'L':
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            break;

        // Solid Triangles
        case 'f':
        case 'F':
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
    }

    // Make the screen update
    glutPostRedisplay();
}

/**
 * main
 */ 
int main (int argc, char **argv)
{
  // Ensure Program has 2 Arguments
  if (argc < 2)
  {  
    printf ("usage: %s heightfield.jpg\n", argv[0]);
    exit(1);
  }

  pic = jpeg_read(argv[1], NULL);
  if (!pic)
  {
    printf ("error reading %s.\n", argv[1]);
    exit(1);
  }

  // Initialize GLUT
  glutInit(&argc,argv);

  // Request double buffer, depth, and color
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);

  // Set window size and poistion
  glutInitWindowSize(640, 480);
  glutInitWindowPosition(0, 0);

  // Create Window
  glutCreateWindow("Height Fields");

  // GLUT Callbacks
  glutDisplayFunc(display);
  glutIdleFunc(doIdle);
  glutMotionFunc(mousedrag);
  glutPassiveMotionFunc(mouseidle);
  glutReshapeFunc(reshape);
  glutMouseFunc(mousebutton);
  glutKeyboardFunc(processKeys);

  // Create right mouse button menu
  g_iMenuId = glutCreateMenu(menufunc);
  glutSetMenu(g_iMenuId);
  glutAddMenuEntry("Quit", 0);
  glutAddMenuEntry("Save Image", 1);
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  // Do Initialization
  init();

  glutMainLoop();
  return(0);
}
