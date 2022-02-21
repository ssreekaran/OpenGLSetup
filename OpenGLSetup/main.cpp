/*******************************************************************
		   Hierarchical Multi-Part Model Example
********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gl/glut.h>
#include "VECTOR3D.h"
#include "cube.h"

const int vWidth = 650;    // Viewport width in pixels
const int vHeight = 500;    // Viewport height in pixels

// Note how everything depends on robot body dimensions so that can scale entire robot proportionately
// just by changing robot body scale
float robotHeadWidth = 2.0;
float robotHeadLength = 2.0;
float robotHeadDepth = 1.8;
float cannonBaseRadius = 0.4;
float cannonBarrelSize = 0.15;
float upperLegLength = 1.5 * robotHeadDepth;
float lowerLegLength = upperLegLength;
float LegWidth = 0.2 * robotHeadWidth;
float wheelDiameter = 1.5 * robotHeadLength;
float wheelWidth = robotHeadWidth;
float wheelTireDepth = 0.5;
float headWidth = 0.4 * robotHeadWidth;
float headLength = headWidth;
float headDepth = headWidth;
float upperArmLength = robotHeadLength;
float upperArmWidth = 0.125 * robotHeadWidth;
float stanchionLength = robotHeadLength;
float stanchionRadius = 0.1 * robotHeadDepth;
float baseWidth = 2 * robotHeadWidth;
float baseLength = 0.25 * stanchionLength;
//Pi
#define M_PI 3.14159265358979323846
//Converts degrees to radians
inline float to_rad(float degrees) {
	return (degrees * M_PI) / 180;
}

// Control Robot body rotation on base
float robotAngle = 0.0;
// Position
float x_pos = cosf(to_rad(robotAngle)) - sinf(to_rad(robotAngle));
float y_pos = sinf(to_rad(robotAngle)) + cosf(to_rad(robotAngle));
// Control arm rotation
float shoulderAngle = -45.0;
float legAngle = 90.0;

// Spin Cube Mesh
float cubeAngle = 0.0;

// Rotate Wheel
float wheelAngle = 0.0;

// Rotate Cannon
float cannonAngle = 0.0;

// Lighting/shading and material properties for robot - upcoming lecture - just copy for now
// Robot RGBA material properties (NOTE: we will learn about this later in the semester)

GLfloat robotBlack_mat_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat robotBlack_mat_diffuse[] = { 0.01f,0.0f,0.01f,0.01f };
GLfloat robotBlack_mat_specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat robotBlack_mat_shininess[] = { 100.0F };

GLfloat robotWhite_mat_ambient[] = { 0.25f, 0.25f, 0.25f, 1.0f };
GLfloat robotWhite_mat_diffuse[] = { 0.4f, 0.4f, 0.4f, 1.0f };
GLfloat robotWhite_mat_specular[] = { 0.774597f, 0.774597f, 0.774597f, 1.0f };
GLfloat robotWhite_mat_shininess[] = { 76.8F };


// Light properties
GLfloat light_position0[] = { -4.0F, 8.0F, 8.0F, 1.0F };
GLfloat light_position1[] = { 4.0F, 8.0F, 8.0F, 1.0F };
GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_ambient[] = { 0.2F, 0.2F, 0.2F, 1.0F };


// Mouse button
int currentButton;

// A template cube mesh
CubeMesh* cubeMesh = createCubeMesh();

// Structure defining a bounding box, currently unused
typedef struct BoundingBox {
	VECTOR3D min;
	VECTOR3D max;
} BBox;

// Prototypes for functions in this module
void initOpenGL(int w, int h);
void display(void);
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void mouseMotionHandler(int xMouse, int yMouse);
void keyboard(unsigned char key, int x, int y);
void functionKeys(int key, int x, int y);
void animationHandler(int param);
void drawRightLeg();
void drawLeftLeg();
void drawRobot();
void drawHead();
void drawWheel();
void drawLeftCannon();
void drawRightCannon();

int main(int argc, char** argv)
{
	// Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(vWidth, vHeight);
	glutInitWindowPosition(200, 30);
	glutCreateWindow("Assignment 1 Bot 4");

	// Initialize GL
	initOpenGL(vWidth, vHeight);

	// Register callback functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotionHandler);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(functionKeys);

	// Start event loop, never returns
	glutMainLoop();

	return 0;
}

// Set up OpenGL. For viewport and projection setup see reshape(). 
void initOpenGL(int w, int h)
{
	// Set up and enable lighting
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);   // This second light is currently off

	// Other OpenGL setup
	glEnable(GL_DEPTH_TEST);   // Remove hidded surfaces
	glShadeModel(GL_SMOOTH);   // Use smooth shading, makes boundaries between polygons harder to see 
	glClearColor(0.4F, 0.4F, 0.4F, 0.0F);  // Color and depth for glClear
	glClearDepth(1.0f);
	glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Nicer perspective

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}

// Callback, called whenever GLUT determines that the window should be redisplayed
// or glutPostRedisplay() has been called.
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	// Create Viewing Matrix V
	// Set up the camera at position (0, 6, 22) looking at the origin, up along positive y axis
	gluLookAt(0.0, 6.0, 22.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	// Draw Robot

	// Apply modelling transformations M to move robot
	// Current transformation matrix is set to IV, where I is identity matrix
	// CTM = IV
	drawRobot();

	// Example of drawing a mesh (closed cube mesh)
	glPushMatrix();
	// spin cube
	glTranslatef(8.0, 0, 3.0);
	glRotatef(cubeAngle, 0.0, 1.0, 0.0);
	glTranslatef(-8.0, 0, -3.0);
	// position and draw cube
	glTranslatef(8.0, 0, 3.0);
	drawCubeMesh(cubeMesh);
	glPopMatrix();

	glutSwapBuffers();   // Double buffering, swap buffers
}

void drawRobot()
{
	glPushMatrix();
	// spin robot on base. 
	glTranslatef(y_pos, 0.0, x_pos);
	glRotatef(45, 0.0, 1.0, 0.0);
	glRotatef(robotAngle, 0.0, 1.0, 0.0);

	// move robot forward or backwards

	drawWheel();
	drawRightLeg();
	drawLeftLeg();
	drawRightCannon();
	drawLeftCannon();
	drawHead();
	//drawLeftArm();
	//drawRightArm();
	glPopMatrix();

	// don't want to spin fixed base
	//drawLowerBody();

	glPopMatrix();
}

void drawHead()
{
	// Set robot material properties per body part. Can have seperate material properties for each part
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotWhite_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotWhite_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotWhite_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotWhite_mat_shininess);

	glPushMatrix();
	// Position head with respect to parent (body)
	glTranslatef(0, robotHeadLength, 0); // this will be done last

	// Build Head
	glPushMatrix();
	GLUquadricObj* myCylinder;
	myCylinder = gluNewQuadric();
	glScalef(robotHeadLength, robotHeadDepth, robotHeadWidth);
	gluQuadricDrawStyle(myCylinder, GLU_FILL);
	glRotatef(90.0, 1.0, 0.0, 0.0);
	gluCylinder(myCylinder, 0.40, 0.40, 1.5f, 20, 20);
	glPopMatrix();

	glPopMatrix();
}

void drawLeftCannon()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotWhite_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotWhite_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotWhite_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotWhite_mat_shininess);

	glPushMatrix();
	// Position arm with respect to parent body
	glTranslatef(0.5 * robotHeadWidth + 0.5 * upperArmWidth, 0, 0.0); // this will be done last

	// build cannon
	glPushMatrix();
	//build sphere base of cannon
	glRotatef(cannonAngle, 1.0, 0.0, 0.0);
	GLUquadricObj* mySphere1;
	mySphere1 = gluNewQuadric();
	gluQuadricDrawStyle(mySphere1, GLU_FILL);
	gluSphere(mySphere1, cannonBaseRadius, 20, 20);
	//build the shooting part of cannon
	glPushMatrix();
	glTranslatef(0.0, 0, cannonBaseRadius);
	GLUquadricObj* myGunBase;
	myGunBase = gluNewQuadric();
	gluQuadricDrawStyle(myGunBase, GLU_FILL);
	gluCylinder(myGunBase, cannonBarrelSize, cannonBarrelSize, 1.5f, 20, 20);
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();
}

void drawRightCannon()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotWhite_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotWhite_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotWhite_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotWhite_mat_shininess);

	glPushMatrix();
	// Position cannon with respect to parent body
	glTranslatef(-(0.5 * robotHeadWidth + 0.5 * upperArmWidth), 0, 0.0); // this will be done last

	// build cannon
	glPushMatrix();
	//build sphere base of cannon
	glRotatef(cannonAngle, 1.0, 0.0, 0.0);
	GLUquadricObj* mySphere1;
	mySphere1 = gluNewQuadric();
	gluQuadricDrawStyle(mySphere1, GLU_FILL);
	gluSphere(mySphere1, cannonBaseRadius, 20, 20);
	//build the shooting part of cannon
	glPushMatrix();
	glTranslatef(0.0, 0, cannonBaseRadius);
	GLUquadricObj* myGunBase;
	myGunBase = gluNewQuadric();
	gluQuadricDrawStyle(myGunBase, GLU_FILL);
	gluCylinder(myGunBase, cannonBarrelSize, cannonBarrelSize, 1.5f, 20, 20);
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();
}

void drawRightLeg()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotWhite_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotWhite_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotWhite_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotWhite_mat_shininess);

	glPushMatrix();

	// Rotate leg at head
	glTranslatef(-(0.4 * robotHeadWidth + 0.5 * LegWidth), 0.35 * upperLegLength, 0.0);
	glRotatef(180, 0.0, 1.0, 0.0);
	glRotatef(shoulderAngle, 1.0, 0.0, 0.0);
	glTranslatef((0.5 * robotHeadWidth + 0.5 * LegWidth), -0.5 * upperLegLength, 0.0);

	// Position leg with respect to head
	glTranslatef(-(0.5 * robotHeadWidth + 0.5 * LegWidth), 0, 0.0);

	// build upper half of leg
	glPushMatrix();
	glScalef(LegWidth, upperLegLength, LegWidth);
	glutSolidCube(1.0);
	glPopMatrix();

	glMaterialfv(GL_FRONT, GL_AMBIENT, robotWhite_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotWhite_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotWhite_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotWhite_mat_shininess);

	glPushMatrix();
	// rotate leg
	glTranslatef(-(0.5 * robotHeadWidth + 0.5 * LegWidth), -(0.5 * lowerLegLength), 0.0);
	glRotatef(legAngle, 1.0, 0.0, 0.0);
	glTranslatef((0.5 * robotHeadWidth + 0.5 * LegWidth), (0.5 * lowerLegLength), 0.0);

	// Position bottom with respect to upper leg 
	glTranslatef(0, -(0.5 * lowerLegLength + 0.5 * lowerLegLength), 0.0);
	// Build lower half of leg
	glScalef(LegWidth, lowerLegLength, LegWidth);
	glutSolidCube(1.0);
	glPopMatrix();

	glPopMatrix();


}

void drawLeftLeg()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotWhite_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotWhite_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotWhite_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotWhite_mat_shininess);

	glPushMatrix();

	// Rotate leg at head
	glTranslatef((0.4 * robotHeadWidth + 0.5 * LegWidth), 0.35 * upperLegLength, 0.0);
	glRotatef(180, 0.0, 1.0, 0.0);
	glRotatef(shoulderAngle, 1.0, 0.0, 0.0);
	glTranslatef(-(0.5 * robotHeadWidth + 0.5 * LegWidth), -0.5 * upperLegLength, 0.0);

	// Position leg with respect to head
	glTranslatef((0.5 * robotHeadWidth + 0.5 * LegWidth), 0, 0.0);

	// build upper half of leg
	glPushMatrix();
	glScalef(LegWidth, upperLegLength, LegWidth);
	glutSolidCube(1.0);
	glPopMatrix();

	glMaterialfv(GL_FRONT, GL_AMBIENT, robotWhite_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotWhite_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotWhite_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotWhite_mat_shininess);

	glPushMatrix();
	// rotate leg
	glTranslatef(-(0.5 * robotHeadWidth + 0.5 * LegWidth), -(0.5 * lowerLegLength), 0.0);
	glRotatef(legAngle, 1.0, 0.0, 0.0);
	glTranslatef((0.5 * robotHeadWidth + 0.5 * LegWidth), (0.5 * lowerLegLength), 0.0);

	// Position bottom with respect to upper leg 
	glTranslatef(0, -(0.5 * lowerLegLength + 0.5 * lowerLegLength), 0.0);

	// build lower half of leg
	glScalef(LegWidth, lowerLegLength, LegWidth);
	glutSolidCube(1.0);
	glPopMatrix();

	glPopMatrix();


}

void drawWheel()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotBlack_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotBlack_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotBlack_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotBlack_mat_shininess);
	//glMaterialfv(GL_FRONT, GL_DIFFUSE, robotArm_mat_diffuse);
	glPushMatrix();
	// Position stanchion and base with respect to body
	glTranslatef(0, -1.5 * robotHeadLength, 0.0); // this will be done last
	glRotatef(wheelAngle, 1.0, 0.0, 0.0);
	// stanchion
	glPushMatrix();
	glScalef(0.8, 0.8, 0.8);
	glRotatef(-90.0, 0.0, 1.0, 0.0);
	glutSolidTorus(((wheelDiameter / 2.0) - wheelTireDepth), (wheelDiameter / 2.0), 20, 20);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotWhite_mat_diffuse);
	glutSolidCube(wheelDiameter/2.0);
	glPushMatrix();
	glTranslatef((wheelDiameter / 2.0)+1, 0.0, 0.0);
	glutSolidCube(0.3);
	glPopMatrix();
	// base
	// Position base with respect to parent stanchion
	glTranslatef(0.0, -0.25 * stanchionLength, 0.0);
	// Build base
	glScalef(baseWidth, baseLength, baseWidth);
	glRotatef(-90.0, 1.0, 0.0, 0.0);
	//glutSolidCube(1.0);
	glPopMatrix();

	glPopMatrix();
}

// Callback, called at initialization and whenever user resizes the window.
void reshape(int w, int h)
{
	// Set up viewport, projection, then change to modelview matrix mode - 
	// display function will then set up camera and do modeling transforms.
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLdouble)w / h, 0.2, 40.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Set up the camera at position (0, 6, 22) looking at the origin, up along positive y axis
	gluLookAt(0.0, 6.0, 22.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

bool stop = false;

// Callback, handles input from the keyboard, non-arrow keys
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'a':
		robotAngle += to_rad(90.0);
		break;
	case 'd':
		robotAngle -= to_rad(90.0);
		break;
	case 'w':
		x_pos += 0.5 * (cosf(to_rad(robotAngle)) - sinf(to_rad(robotAngle)));
		y_pos += 0.5 * (sinf(to_rad(robotAngle)) +  cosf(to_rad(robotAngle)));
		wheelAngle += 7.0;
		break;
	case 's':
		x_pos -= 0.5 * (cosf(to_rad(robotAngle)) - sinf(to_rad(robotAngle)));
		y_pos -= 0.5 * (sinf(to_rad(robotAngle)) + cosf(to_rad(robotAngle)));
		wheelAngle -= 7.0;
		break;
	case 'q':
		glutTimerFunc(10, animationHandler, 0);
		break;
	case 'Q':
		stop = true;
		break;
	}

	glutPostRedisplay();   // Trigger a window redisplay
}

void animationHandler(int param)
{
	if (!stop)
	{
		cubeAngle += 2.0;
		glutPostRedisplay();
		glutTimerFunc(10, animationHandler, 0);
	}
}



// Callback, handles input from the keyboard, function and arrow keys
void functionKeys(int key, int x, int y)
{
	// Help key
	switch (key)
	{
	case GLUT_KEY_UP:
		if (cannonAngle > -65)
			cannonAngle -= 1.0;
		break;
	case GLUT_KEY_DOWN:
		if (cannonAngle < 65)
			cannonAngle += 1.0;
		break;
	case GLUT_KEY_F1:
		printf("Move Forward: w\nMove Backward: s\nMove Left: a\nMove Right: d\nRaise Cannons: Up Arrow Key\nLower Cannons: Down Arrow Key\nAnimate Cube: q\nStop Animation: Q");
		break;
	}
	// Do transformations with arrow keys
	//else if (...)   // GLUT_KEY_DOWN, GLUT_KEY_UP, GLUT_KEY_RIGHT, GLUT_KEY_LEFT
	//{
	//}

	glutPostRedisplay();   // Trigger a window redisplay
}


// Mouse button callback - use only if you want to 
void mouse(int button, int state, int x, int y)
{
	currentButton = button;

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN)
		{
			;

		}
		break;
	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN)
		{
			;
		}
		break;
	default:
		break;
	}

	glutPostRedisplay();   // Trigger a window redisplay
}


// Mouse motion callback - use only if you want to 
void mouseMotionHandler(int xMouse, int yMouse)
{
	if (currentButton == GLUT_LEFT_BUTTON)
	{
		;
	}

	glutPostRedisplay();   // Trigger a window redisplay
}
