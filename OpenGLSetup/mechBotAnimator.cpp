#include <stdio.h>
#include <windows.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <math.h>
#include <string.h>
#include "mechBotAnimator.h"
#include "subdivcurve.h"

//enum BotType { CUBE, SPHERE, WHEEL, MINE};
//BotType botType = MINE;

int numCirclePoints = 30;
double circleRadius = 0.2;
int hoveredCircle = -1;
int curveIndex = 0;
int currentCurvePoint = 0;
int angle = 0;
int animate = 0;
int delay = 15; // milliseconds

void drawRightLeg();
void drawLeftLeg();
void drawRobot2();
void drawRobot3();
void drawHead();
void drawWheel();
void drawLeftCannon();
void drawRightCannon();
void drawLeftBotCannon();
void drawRightBotCannon();
void drawBot();
void drawBot2();
void drawBrokenCannon(); 
void drawProjectile2();
void drawProjectile3();

double recAngle = 0;
double wheelAngle = 0;
double cannonXDir = 0;
double cannonXPos = 0.0;
double cannonZPos = 7.0;
double deadCannonX = 0;

GLdouble worldLeft = -12;
GLdouble worldRight = 12;
GLdouble worldBottom = -9;
GLdouble worldTop = 9;
GLdouble worldCenterX = 0.0;
GLdouble worldCenterY = 0.0;
GLdouble wvLeft = -12;
GLdouble wvRight = 12;
GLdouble wvBottom = -9;
GLdouble wvTop = 9;

GLint glutWindowWidth = 800;
GLint glutWindowHeight = 600;
GLint viewportWidth = glutWindowWidth;
GLint viewportHeight = glutWindowHeight;

// Ground Mesh material
GLfloat groundMat_ambient[]    = {0.4, 0.4, 0.4, 1.0};
GLfloat groundMat_specular[]   = {0.01, 0.01, 0.01, 1.0};
GLfloat groundMat_diffuse[]   = {0.4, 0.4, 0.7, 1.0};
GLfloat groundMat_shininess[]  = {1.0};

GLfloat light_position0[] = {4.0, 8.0, 8.0, 1.0};
GLfloat light_diffuse0[] = {1.0, 1.0, 1.0, 1.0};

GLfloat light_position1[] = {-4.0, 8.0, 8.0, 1.0};
GLfloat light_diffuse1[] = {1.0, 1.0, 1.0, 1.0};

GLfloat light_specular[] = {1.0, 1.0, 1.0, 1.0};
GLfloat model_ambient[]  = {0.5, 0.5, 0.5, 1.0};

// 
GLdouble spin  = 0.0;

// The 2D animation path curve is a subdivision curve
SubdivisionCurve subcurve;
GLdouble posX = 0;
GLdouble posZ = 0;
int posOnCurve = 0;
int anglePos = 1;

double xChange = 0.0;
double zChange = 4.0;
double xPos = 5.0 + xChange;
double zPos = 7.0 + zChange;
double leftCannonAim =  180- atan(xPos / zPos) * (180 / M_PI);
double rightCannonAim =  180+ atan((xPos) / (zPos)) * (180 / M_PI);
double proj2x = 0;
double proj2z = 0;
double proj3 = 0;
bool visible1 = true;
bool visible2 = true;
bool unbroken = true;
bool mechFiring = true;
bool mechFiring2 = false;
bool hit = false;
bool firstPersonView = false;
double defencePosition = 0;
double defenceBullet1PositionX = 0;
double defenceBullet1PositionZ = 0;
double defenceBullet1PositionVec = 0;
int bulletCounter = 0;
double die = 1;

// Use circles to **draw** (i.e. visualize) subdivision curve control points
Circle circles[MAXCONTROLPOINTS];

int lastMouseX;
int lastMouseY;
int window2D, window3D;
int window3DSizeX = 800, window3DSizeY = 600;
GLdouble aspect = (GLdouble)window3DSizeX / window3DSizeY;
GLdouble eyeX = 0.0, eyeY = 6.0, eyeZ = 25.0;
GLdouble zNear = 0.1, zFar = 40.0;
GLdouble fov = 60.0;

int main(int argc, char* argv[])
{
	//printf("Value of x = %f\n", xPos);
	//printf("Value of z = %f\n", zPos);
	glutInit(&argc, (char **)argv); 
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(glutWindowWidth,glutWindowHeight);
	glutInitWindowPosition(50,100);  
	
	// The 2D Window
	window2D = glutCreateWindow("Animation Path Designer"); 
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	// Initialize the 2D profile curve system
	init2DCurveWindow(); 
	// A few input handlers
	glutMouseFunc(mouseButtonHandler);
	glutMotionFunc(mouseMotionHandler);
	glutPassiveMotionFunc(mouseHoverHandler);
	glutMouseWheelFunc(mouseScrollWheelHandler);
	glutKeyboardFunc(keyboardHandler);
	glutSpecialFunc(specialKeyHandler);
	

	// The 3D Window
	window3D = glutCreateWindow("Mech Bot"); 
	glutPositionWindow(900,100);  
	glutDisplayFunc(display3D);
	glutReshapeFunc(reshape3D);
	glutMouseFunc(mouseButtonHandler3D);
	glutMouseWheelFunc(mouseScrollWheelHandler3D);
	glutMotionFunc(mouseMotionHandler3D);
	glEnable(GL_DEPTH_TEST);
	// Initialize the 3D system
	init3DSurfaceWindow();

	// Annnd... ACTION!!
	glutMainLoop(); 



	return 0;
}

void assignColor(GLfloat col[3], GLfloat r, GLfloat g, GLfloat b) {
	col[0] = r;
	col[1] = g;
	col[2] = b;
}

int isEven(int x) {
	if (x % 2 == 0)
		return 1;
	else
		return 0;
}

GLfloat textureMap[64][64][3];
GLuint tex[2];

void makeTextureMap()
{
	for (int i = 0; i < 64; i++)
		for (int j = 0; j < 64; j++) {
			if (isEven(i / 8 + j / 8))
				assignColor(textureMap[i][j], 1.0, 1.0, 1.0);
			else
				assignColor(textureMap[i][j], 1.0, 0.0, 0.0);
		}
}
void makeTextures()
{
	GLfloat planes[] = { 0.0, 0.0, 0.3, 0.0 };
	GLfloat planet[] = { 0.0, 0.3, 0.0, 0.0 };

	glGenTextures(2, tex);

	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_FLOAT, textureMap);

	/************************************ */
	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenfv(GL_S, GL_OBJECT_PLANE, planes);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenfv(GL_T, GL_OBJECT_PLANE, planet);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_FLOAT, textureMap);
}

//Converts degrees to radians
inline float to_rad(double degrees) {
	return (degrees * M_PI) / 180;
}

inline float to_deg(double radians) {
	return (radians * 180) / M_PI;
}
void init2DCurveWindow() 
{ 
	glLineWidth(3.0);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glClearColor(0.4F, 0.4F, 0.4F, 0.0F);
	initSubdivisionCurve();
	initControlPoints();
} 

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	
	glMatrixMode(GL_PROJECTION);	
	glLoadIdentity();
	gluOrtho2D(wvLeft, wvRight, wvBottom, wvTop);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	draw2DScene();	
	glutSwapBuffers();
}


void draw2DScene() 
{
	drawAxes();
	drawSubdivisionCurve();
	drawControlPoints();
}

void drawAxes()
{
	glPushMatrix();
	glColor3f(1.0, 0.0, 0);
	glBegin(GL_LINE_STRIP);
	glVertex3f(0, 8.0, 0);
	glVertex3f(0, -8.0, 0);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3f(-8, 0.0, 0);
	glVertex3f(8, 0.0, 0);
	glEnd();
	glPopMatrix();
}

void drawSubdivisionCurve() {
	// Subdivide the given curve
	computeSubdivisionCurve(&subcurve);
	
	int i=0;

	glColor3f(0.0, 1.0, 0.0);
	glPushMatrix();
	glBegin(GL_LINE_STRIP);
	for (i=0; i<subcurve.numCurvePoints; i++){
		glVertex3f(subcurve.curvePoints[i].x, subcurve.curvePoints[i].y, 0.0);
	}
	glEnd();
	glPopMatrix();
}

void drawControlPoints(){
	int i, j;
	for (i=0; i<subcurve.numControlPoints; i++){
		glPushMatrix();
		glColor3f(1.0f,0.0f,0.0f); 
		glTranslatef(circles[i].circleCenter.x, circles[i].circleCenter.y, 0);
		// for the hoveredCircle, draw an outline and change its colour
		if (i == hoveredCircle){ 
			// outline
			glColor3f(0.0, 1.0, 0.0);
			glBegin(GL_LINE_LOOP); 
			for(j=0; j < numCirclePoints; j++) {
				glVertex3f(circles[i].circlePoints[j].x, circles[i].circlePoints[j].y, 0); 
			}
			glEnd();
			// colour change
			glColor3f(0.5,0.0,1.0);
		}
		glBegin(GL_LINE_LOOP); 
		for(j=0; j < numCirclePoints; j++) {
			glVertex3f(circles[i].circlePoints[j].x, circles[i].circlePoints[j].y, 0); 
		}
		glEnd();
		glPopMatrix();
	}
}

void initSubdivisionCurve() {
	// Initialize 3 control points of the subdivision curve

	GLdouble x, y;

	x = 4 * cos(M_PI*0.5);
	y = 4 * sin(M_PI*0.5);
	subcurve.controlPoints[0].x = x;
	subcurve.controlPoints[0].y = y;

	x = 4 * cos(M_PI*0.25);
	y = 4 * sin(M_PI*0.25);
	subcurve.controlPoints[1].x = x;
	subcurve.controlPoints[1].y = y;

	x = 4 * cos(M_PI*0.0);
	y = 4 * sin(M_PI*0.0);
	subcurve.controlPoints[2].x = x;
	subcurve.controlPoints[2].y = y;

	x = 4 * cos(-M_PI*0.25);
	y = 4 * sin(-M_PI*0.25);
	subcurve.controlPoints[3].x = x;
	subcurve.controlPoints[3].y = y;

	x = 4 * cos(-M_PI * 0.5);
	y = 4 * sin(-M_PI * 0.5);
	subcurve.controlPoints[4].x = x;
	subcurve.controlPoints[4].y = y;

	x = 4 * cos(-M_PI * 0.25);
	y = 4 * sin(-M_PI * 0.25);
	subcurve.controlPoints[5].x = x;
	subcurve.controlPoints[5].y = y;

	x = 4 * cos(M_PI * 0.0);
	y = 4 * sin(M_PI * 0.0);
	subcurve.controlPoints[6].x = x;
	subcurve.controlPoints[6].y = y;

	x = 4 * cos(M_PI * 0.25);
	y = 4 * sin(M_PI * 0.25);
	subcurve.controlPoints[7].x = x;
	subcurve.controlPoints[7].y = y;

	x = 4 * cos(M_PI * 0.5);
	y = 4 * sin(M_PI * 0.5);
	subcurve.controlPoints[8].x = x;
	subcurve.controlPoints[8].y = y;

	subcurve.numControlPoints = 9;
	subcurve.subdivisionSteps = 4;
}

void initControlPoints(){
	int i;
	int num = subcurve.numControlPoints;
	for (i=0; i < num; i++){
		constructCircle(circleRadius, numCirclePoints, circles[i].circlePoints);
		circles[i].circleCenter = subcurve.controlPoints[i];
	}
}

void screenToWorldCoordinates(int xScreen, int yScreen, GLdouble *xw, GLdouble *yw)
{
	GLdouble xView, yView;
	screenToCameraCoordinates(xScreen, yScreen, &xView, &yView);
	cameraToWorldCoordinates(xView, yView, xw, yw);
}

void screenToCameraCoordinates(int xScreen, int yScreen, GLdouble *xCamera, GLdouble *yCamera)
{
	*xCamera = ((wvRight-wvLeft)/glutWindowWidth)  * xScreen; 
	*yCamera = ((wvTop-wvBottom)/glutWindowHeight) * (glutWindowHeight-yScreen); 
}

void cameraToWorldCoordinates(GLdouble xcam, GLdouble ycam, GLdouble *xw, GLdouble *yw)
{
	*xw = xcam + wvLeft;
	*yw = ycam + wvBottom;
}

void worldToCameraCoordiantes(GLdouble xWorld, GLdouble yWorld, GLdouble *xcam, GLdouble *ycam)
{
	double wvCenterX = wvLeft   + (wvRight - wvLeft)/2.0;
	double wvCenterY = wvBottom + (wvTop   - wvBottom)/2.0;
	*xcam = worldCenterX - wvCenterX + xWorld;
	*ycam = worldCenterY - wvCenterY + yWorld;
}

int currentButton;

void mouseButtonHandler(int button, int state, int xMouse, int yMouse)
{
	int i;
	
	currentButton = button;
	if (button == GLUT_LEFT_BUTTON)
	{  
		switch (state) {      
		case GLUT_DOWN:
			if (hoveredCircle > -1) {
				screenToWorldCoordinates(xMouse, yMouse, &circles[hoveredCircle].circleCenter.x, &circles[hoveredCircle].circleCenter.y);
				screenToWorldCoordinates(xMouse, yMouse, &subcurve.controlPoints[hoveredCircle].x, &subcurve.controlPoints[hoveredCircle].y);
			}
			break;
		case GLUT_UP:
			glutSetWindow(window3D);
			glutPostRedisplay();
			break;
		}
	}    
	else if (button == GLUT_MIDDLE_BUTTON)
	{
		switch (state) {      
		case GLUT_DOWN:
			break;
		case GLUT_UP:
			if (hoveredCircle == -1 && subcurve.numControlPoints < MAXCONTROLPOINTS){ 
				GLdouble newPointX;
				GLdouble newPointY;
				screenToWorldCoordinates(xMouse, yMouse, &newPointX, &newPointY);
				subcurve.controlPoints[subcurve.numControlPoints].x = newPointX;
				subcurve.controlPoints[subcurve.numControlPoints].y = newPointY;			
				constructCircle(circleRadius, numCirclePoints, circles[subcurve.numControlPoints].circlePoints);
				circles[subcurve.numControlPoints].circleCenter = subcurve.controlPoints[subcurve.numControlPoints];
				subcurve.numControlPoints++;
			} else if (hoveredCircle > -1 && subcurve.numControlPoints > MINCONTROLPOINTS) {
				subcurve.numControlPoints--;
				for (i=hoveredCircle; i<subcurve.numControlPoints; i++){
					subcurve.controlPoints[i].x = subcurve.controlPoints[i+1].x;
					subcurve.controlPoints[i].y = subcurve.controlPoints[i+1].y;
					circles[i].circleCenter = circles[i+1].circleCenter;
				}
			}
			
			glutSetWindow(window3D);
			glutPostRedisplay();
			break;
		}
	}

	glutSetWindow(window2D);
	glutPostRedisplay();
}

void mouseMotionHandler(int xMouse, int yMouse)
{
	if (currentButton == GLUT_LEFT_BUTTON) {  
		if (hoveredCircle > -1) {
			screenToWorldCoordinates(xMouse, yMouse, &circles[hoveredCircle].circleCenter.x, &circles[hoveredCircle].circleCenter.y);
			screenToWorldCoordinates(xMouse, yMouse, &subcurve.controlPoints[hoveredCircle].x, &subcurve.controlPoints[hoveredCircle].y);
		}
	}    
	else if (currentButton == GLUT_MIDDLE_BUTTON) {
	}
	glutPostRedisplay();
}

void mouseHoverHandler(int xMouse, int yMouse)
{
	hoveredCircle = -1;
	GLdouble worldMouseX, worldMouseY;
	screenToWorldCoordinates(xMouse, yMouse, &worldMouseX, &worldMouseY);
	int i;
	// see if we're hovering over a circle
	for (i=0; i<subcurve.numControlPoints; i++){
		GLdouble distToX = worldMouseX - circles[i].circleCenter.x;
		GLdouble distToY = worldMouseY - circles[i].circleCenter.y;
		GLdouble euclideanDist = sqrt(distToX*distToX + distToY*distToY);
		//printf("Dist from point %d is %.2f\n", i, euclideanDist);
		if (euclideanDist < 2.0){
			hoveredCircle = i;
		}
	}
	
	glutPostRedisplay();
}

void mouseScrollWheelHandler(int button, int dir, int xMouse, int yMouse)
{
	

	glutPostRedisplay();

}
bool start = true;
bool shoot = false;
void keyboardHandler(unsigned char key, int x, int y)
{
	switch(key){
	case 'q':
	case 'Q':
	case 27:
		// Esc, q, or Q key = Quit 
		exit(0);
		break;
	case 'a':
		// Add code to create timer and call animation handler
		start = true;
		// Use this to set to 3D window and redraw it
		glutTimerFunc(10, animationHandler, 0);
		break;
	case 'r':
		// reset object position at beginning of curve
		start = false;
		shoot = false;
		visible1 = true;
		visible2 = true;
		unbroken = true;
		proj3 = 0;
		glutSetWindow(window3D);
		glutPostRedisplay();
		//.......
		break;
	case 'f' :
		if (firstPersonView) {
			firstPersonView = false;
		}
		else {
			firstPersonView = true;
		}
		break;
	case 32:
		shoot = true;
		default:
		break;
	}
	glutPostRedisplay();
}

void specialKeyHandler(int key, int x, int y)
{
	switch(key)	{
	case GLUT_KEY_LEFT:
		// add code here
		if (cannonXPos >= -9) {
			cannonXPos -= 0.5;
		}
		glutSetWindow(window3D);
		glutPostRedisplay();
		break;
	case GLUT_KEY_RIGHT:
		// add code here;
		if (cannonXPos <= 11) {
			cannonXPos += 0.5;
		}
		glutSetWindow(window3D);
		glutPostRedisplay();
		break;
	case GLUT_KEY_F1:
		printf("Move Left: Left Arrow\nMove Right: Right\nStart: a\nRestart: r\nToggle First Person Mode: f\nShoot: Spacebar\n\nDestroy the two Robots before it destroys you.");
		break;
	}
	glutPostRedisplay();
}


void reshape(int w, int h)
{
	glutWindowWidth = (GLsizei) w;
	glutWindowHeight = (GLsizei) h;
	glViewport(0, 0, glutWindowWidth, glutWindowHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(wvLeft, wvRight, wvBottom, wvTop);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}



/************************************************************************************
 *
 *
 * 3D Window Code 
 *
 * Fill in the code in the empty functions
 ************************************************************************************/



void init3DSurfaceWindow()
{
	glEnable(GL_TEXTURE_2D);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse0);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_AMBIENT, model_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse1);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_AMBIENT, model_ambient);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	makeTextureMap();
	makeTextures();
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
	glClearColor(0.4F, 0.4F, 0.4F, 0.0F);  // Color and depth for glClear

	glViewport(0, 0, (GLsizei)window3DSizeX, (GLsizei)window3DSizeY);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, aspect, zNear, zFar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, 6.0, 22.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}


void reshape3D(int w, int h)
{
	glutWindowWidth = (GLsizei) w;
	glutWindowHeight = (GLsizei) h;
	glViewport(0, 0, glutWindowWidth, glutWindowHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov,aspect,zNear,zFar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, 6.0, 22.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}
double angleBtwnTwoVec(double v1, double v2, double v3, double v4) {
	return to_deg(acos((v1 * v3 + v2 * v4) / (sqrt(pow(v1, 2) + pow(v2, 2)))));
}

double angleBtwnVecXAxis(double v1, double v2) {
	if (v1 >= 0 && v2 >= 0) {
		return angleBtwnTwoVec(v1, v2, 1.0, 0.0);
	}
	else if (v1 <= 0 && v2 >= 0) {
		return 90.0 + angleBtwnTwoVec(v1, v2, 0.0, 1.0);
	}
	else if (v1 <= 0 && v2 <= 0) {
		return 180.0 + angleBtwnTwoVec(v1, v2,  -1.0, 0.0);
	}
	else {
		return 270.0 + angleBtwnTwoVec(v1, v2, 0.0, -1.0);
	}
}
void animationHandler(int param)
{
	double vec1x;
	double vec1z;
	double vec2x;
	double vec2z;
	double vec3x;
	double vec3z;
	if (start) {
		if (posOnCurve < subcurve.numCurvePoints-1) {
			posOnCurve += 1;
			if (posOnCurve < subcurve.numCurvePoints - 2) {
				if (shoot) {
					proj3 += 1;
					if (proj3 > 30) {
						proj3 = 0;
						shoot = false;
					}

				}

				anglePos += 1;
				wheelAngle += 10;
				xChange = subcurve.curvePoints[anglePos].x;
				zChange = subcurve.curvePoints[anglePos].y;
				xPos = 5.0 + xChange;
				zPos = 7.0 + zChange;
				posX = 4.0 + subcurve.curvePoints[posOnCurve].x;
				posZ = -subcurve.curvePoints[posOnCurve].y;
				vec1x = cannonXPos - posX;
				vec1z = cannonZPos - posZ;
				vec2x = sinf(to_rad(recAngle));
				vec2z = cosf(to_rad(recAngle));
				vec3x = cannonXPos - posX + 10;
				vec3z = cannonZPos - posZ;

				leftCannonAim = angleBtwnVecXAxis(vec2x, vec2z) - angleBtwnVecXAxis(vec3x, vec3z);
				rightCannonAim = angleBtwnVecXAxis(vec2x, vec2z) - angleBtwnVecXAxis(vec1x, vec1z);
				if (posX > defencePosition + 2.5 && posX < defencePosition + 4.5 && posZ > cannonZPos - proj3 -1 && posZ < cannonZPos - proj3 + 1) {
					visible2 = false;
				}
				if (posX - 10 > defencePosition + 2.5 && posX - 10 < defencePosition + 4.5 && posZ > cannonZPos - proj3 - 1 && posZ < cannonZPos - proj3 + 1) {
					visible1 = false;
				}
				if (defencePosition + 1.0 < defenceBullet1PositionX && defencePosition + 3.0 > defenceBullet1PositionX && defenceBullet1PositionZ < cannonZPos && cannonZPos < defenceBullet1PositionZ + 2.0) {
					unbroken = false;
				}

				if (firstPersonView) {
					eyeX = -1.0 + cannonXPos;
					eyeY = 1.0;
					eyeZ = 10.0;
				}
				else {
					eyeX = 0.0, eyeY = 6.0, eyeZ = 25.0;
				}
			}
			else {
				posOnCurve = 0;
				anglePos = 1;
			}
		}
		glutSetWindow(window3D);
		glutPostRedisplay();
		glutTimerFunc(100, animationHandler, 0);
	}

	if (!start) {
		posOnCurve = 0;
		anglePos = 1;
	}
}

void display3D()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(eyeX, eyeY, eyeZ, eyeX, 0.0, 0.0, 0.0, 1.0, 0.0);
	drawGround();
	glTranslatef(-1,0,4);
	glPushMatrix();
	//draw3DSubdivisionCurve();
	//draw3DControlPoints();
	glPushMatrix();
	glTranslatef(-5, 0, 0);
	if (visible1) {
		drawBot();
	}
	glPopMatrix();
	glPushMatrix();
	glTranslatef(5, 0, 0);
	if (visible2) {
		drawBot2();
	}
	drawProjectile2();
	if (unbroken)
		drawProjectile3();
	glPopMatrix();
	if (unbroken)
		drawCannon();
	else
		drawBrokenCannon();
	glutSwapBuffers();
	glPopMatrix();
}

void draw3DSubdivisionCurve() 
{
	computeSubdivisionCurve(&subcurve);

	int i = 0;

	glColor3f(0.0, 1.0, 0.0);
	glPushMatrix();
	glRotatef(-90, 1, 0, 0);
	glBegin(GL_LINE_STRIP);
	for (i = 0; i < subcurve.numCurvePoints; i++) {
		glVertex3f(subcurve.curvePoints[i].x, subcurve.curvePoints[i].y, 0.0);
	}
	glEnd();
	glPopMatrix();
}

void draw3DControlPoints()
{
	int i, j;
	for (i = 0; i < subcurve.numControlPoints; i++) {
		glPushMatrix();
		glColor3f(1.0f, 0.0f, 0.0f);
		glRotatef(-90, 1, 0, 0);
		glTranslatef(circles[i].circleCenter.x, circles[i].circleCenter.y, 0);
		// for the hoveredCircle, draw an outline and change its colour
		if (i == hoveredCircle) {
			// outline
			glColor3f(0.0, 1.0, 0.0);
			glBegin(GL_LINE_LOOP);
			for (j = 0; j < numCirclePoints; j++) {
				glVertex3f(circles[i].circlePoints[j].x, circles[i].circlePoints[j].y, 0);
			}
			glEnd();
			// colour change
			glColor3f(0.5, 0.0, 1.0);
		}
		glBegin(GL_LINE_LOOP);
		for (j = 0; j < numCirclePoints; j++) {
			glVertex3f(circles[i].circlePoints[j].x, circles[i].circlePoints[j].y, 0);
		}
		glEnd();
		glPopMatrix();
	}
}

GLfloat robotBody_mat_ambient[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat robotBody_mat_specular[] = { 0.45f,0.55f,0.45f,1.0f };
GLfloat robotBody_mat_diffuse[] = { 0.1f,0.35f,0.1f,1.0f };
GLfloat robotBody_mat_shininess[] = { 20.0F };

void drawBot()
{
	glPushMatrix();
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, robotBody_mat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, robotBody_mat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, robotBody_mat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, robotBody_mat_shininess);
	posX = subcurve.curvePoints[posOnCurve].x;
	posZ = -subcurve.curvePoints[posOnCurve].y;
	glTranslatef(posX, 0.0, posZ);
	recAngle = -atan((subcurve.curvePoints[anglePos].x - subcurve.curvePoints[anglePos - 1].x) / (subcurve.curvePoints[anglePos].y - subcurve.curvePoints[anglePos - 1].y)) * 180 / M_PI;
	glRotatef(90, 0, 1, 0);
	glRotatef(recAngle, 0, 1, 0);
	glPushMatrix();
	glTranslatef(0, 1, 0);
	glScalef(0.4, 0.4, 0.4);
	glRotatef(-135, 0, 1, 0);
	drawRobot2();
	glPopMatrix();
}

void drawBot2()
{
	glPushMatrix();
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, robotBody_mat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, robotBody_mat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, robotBody_mat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, robotBody_mat_shininess);
	posX = subcurve.curvePoints[posOnCurve].x;
	posZ = -subcurve.curvePoints[posOnCurve].y;
	glTranslatef(posX, 0.0, posZ);
	recAngle = -atan((subcurve.curvePoints[anglePos].x - subcurve.curvePoints[anglePos - 1].x) / (subcurve.curvePoints[anglePos].y - subcurve.curvePoints[anglePos - 1].y)) * 180 / M_PI;
	glRotatef(90, 0, 1, 0);
	glRotatef(recAngle, 0, 1, 0);
	glPushMatrix();
	glTranslatef(0, 1, 0);
	glScalef(0.4, 0.4, 0.4);
	glRotatef(-135, 0, 1, 0);
	drawRobot3();
	glPopMatrix();
}


void drawCannon() 
{
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glPushMatrix();
	glTranslatef(cannonXPos, 0.0, cannonZPos);
	glTranslatef(cannonXDir, 0.0, 0.0);
	GLUquadricObj* mySphere;
	mySphere = gluNewQuadric();
	gluQuadricDrawStyle(mySphere, GLU_FILL);
	glPolygonMode(GL_FRONT, GL_FILL);
	gluQuadricNormals(mySphere, GLU_SMOOTH);
	gluSphere(mySphere, 0.75, 20, 20);
	glTranslatef(0.0, 0.75, 0.0);
	GLUquadricObj* mySphere2;
	mySphere2 = gluNewQuadric();
	gluQuadricDrawStyle(mySphere2, GLU_FILL);
	glPolygonMode(GL_FRONT, GL_FILL);
	gluQuadricNormals(mySphere2, GLU_SMOOTH);
	gluSphere(mySphere2, 0.5, 20, 20);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glPopMatrix();
	glTranslatef(cannonXPos, 0.0, cannonZPos);
	glTranslatef(cannonXDir, 0.0, 0.0);
	glTranslatef(0.0, 0.75, 0.0);
	deadCannonX = cannonXPos;
}

void drawBrokenCannon()
{
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glPushMatrix();
	glTranslatef(deadCannonX, 0.0, cannonZPos);
	glTranslatef(cannonXDir, 0.0, 0.0);
	GLUquadricObj* mySphere;
	mySphere = gluNewQuadric();
	gluQuadricDrawStyle(mySphere, GLU_FILL);
	glPolygonMode(GL_FRONT, GL_FILL);
	gluQuadricNormals(mySphere, GLU_SMOOTH);
	gluSphere(mySphere, 0.75, 20, 20);
	glTranslatef(0.75, 0.0, 0.0);
	GLUquadricObj* mySphere2;
	mySphere2 = gluNewQuadric();
	gluQuadricDrawStyle(mySphere2, GLU_FILL);
	glPolygonMode(GL_FRONT, GL_FILL);
	gluQuadricNormals(mySphere2, GLU_SMOOTH);
	gluSphere(mySphere2, 0.5, 20, 20);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glPopMatrix();
	glTranslatef(deadCannonX, 0.0, cannonZPos);
	glTranslatef(cannonXDir, 0.0, 0.0);
	glTranslatef(0.0, 0.75, 0.0);
}

void drawGround() {
	glPushMatrix();
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, groundMat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, groundMat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, groundMat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, groundMat_shininess);
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);
	glVertex3f(-12.0f, -1.0f, -12.0f);
	glVertex3f(-12.0f, -1.0f, 12.0f);
	glVertex3f(12.0f, -1.0f, 12.0f);
	glVertex3f(12.0f, -1.0f, -12.0f);
	glEnd();
	glPopMatrix();
}

void mouseButtonHandler3D(int button, int state, int x, int y)
{

	currentButton = button;
	lastMouseX = x;
	lastMouseY = y;
	switch(button)
	{
	case GLUT_LEFT_BUTTON:
		//eyeZ -= 0.5;
		break;
	case GLUT_RIGHT_BUTTON:
		//eyeZ += 0.5;
		break;
	case GLUT_MIDDLE_BUTTON:
		
		break;
	default:
		break;
	}
}

void mouseScrollWheelHandler3D(int button, int dir, int xMouse, int yMouse)
{
	if (button > 0) {
		
	}
	if (dir>0) {

	}
	if (dir < 0) {

	}
}

void mouseMotionHandler3D(int x, int y)
{
	int dx = x - lastMouseX;
	int dy = y - lastMouseY;
	if (currentButton == GLUT_LEFT_BUTTON) {
		//eyeZ += (dy/15);
	}
	if (currentButton == GLUT_RIGHT_BUTTON) 
	{
		;
	}
	else if (currentButton == GLUT_MIDDLE_BUTTON) 
	{
	}
	lastMouseX = x;
	lastMouseY = y;
	glutPostRedisplay();
}



// Some Utility Functions

Vector3D crossProduct(Vector3D a, Vector3D b){
	Vector3D cross;
	
	cross.x = a.y * b.z - b.y * a.z;
	cross.y = a.x * b.z - b.x * a.z;
	cross.z = a.x * b.y - b.x * a.y;
	
	return cross;
}

Vector3D normalize(Vector3D a){
	GLdouble norm = sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
	Vector3D normalized;
	normalized.x = a.x/norm;
	normalized.y = a.y/norm;
	normalized.z = a.z/norm;
	return normalized;
}

//Bonus
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
//float wheelAngle2 = 0.0;

// Rotate Cannon
float cannonAngle = 0.0;

int temp = 0;
bool temp2 = true;
void drawProjectile2() {
	glPushMatrix();
	if (!mechFiring2) {
		if (temp == 30) {
			mechFiring2 = true;
			temp = 0;
		}
		if (visible1 && visible2) {
			if (temp2) {
				defenceBullet1PositionX = posX;
				defenceBullet1PositionZ = posZ;
				defenceBullet1PositionVec = recAngle + rightCannonAim;
			}
			else {
				defenceBullet1PositionX = posX - 10;
				defenceBullet1PositionZ = posZ;
				defenceBullet1PositionVec = recAngle + leftCannonAim;
			}
		}
		else if (!visible1 && visible2) {
			defenceBullet1PositionX = posX;
			defenceBullet1PositionZ = posZ;
			defenceBullet1PositionVec = recAngle + rightCannonAim;
		}
		else if (visible1 && !visible2) {
			defenceBullet1PositionX = posX - 10;
			defenceBullet1PositionZ = posZ;
			defenceBullet1PositionVec = recAngle + leftCannonAim;
		}
		else {

		}
	}
	else {
		defenceBullet1PositionX += sinf(to_rad(defenceBullet1PositionVec));
		defenceBullet1PositionZ += cosf(to_rad(defenceBullet1PositionVec));
		if (temp == 30) {
			mechFiring2 = false;
			temp = 0;
			if (temp2)
				temp2 = false;
			else
				temp2 = true;
		}
	}
	temp++;
	glTranslatef(defenceBullet1PositionX,1, defenceBullet1PositionZ);
	glRotatef(defenceBullet1PositionVec,0,1,0);
	glutSolidCone(0.25, 1, 20, 20);
	glPopMatrix();
}

void drawProjectile3() {
	glPushMatrix();
	if (!shoot) {
		glTranslatef(cannonXPos-5, 0.0, cannonZPos);
		defencePosition = cannonXPos-5;
	}
	else if (shoot){
		glTranslatef(defencePosition, 0, cannonZPos -proj3);
	}
	glRotatef(180,1,0,0);
	glutSolidCone(0.25, 1, 20, 20);
	glPopMatrix();
}

void drawRobot2()
{
	glPushMatrix();
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glBindTexture(GL_TEXTURE_2D, tex[1]);
	// spin robot on base. 
	glTranslatef(y_pos, 0.0, x_pos);
	glRotatef(45, 0.0, 1.0, 0.0);
	glRotatef(robotAngle, 0.0, 1.0, 0.0);

	// move robot forward or backwards

	drawWheel();
	drawRightLeg();
	drawLeftLeg();
	drawHead();
	drawLeftBotCannon();
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glPopMatrix();
	glPopMatrix();
}

void drawRobot3()
{
	glPushMatrix();
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	// spin robot on base. 
	glTranslatef(y_pos, 0.0, x_pos);
	glRotatef(45, 0.0, 1.0, 0.0);
	glRotatef(robotAngle, 0.0, 1.0, 0.0);

	// move robot forward or backwards

	drawWheel();
	drawRightLeg();
	drawLeftLeg();
	drawHead();
	drawRightBotCannon();
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glPopMatrix();
	glPopMatrix();
}

void drawLeftBotCannon() {
	glPushMatrix();
	glRotatef(leftCannonAim, 0, 1, 0);
	drawRightCannon();
	//drawProjectile1();
	drawLeftCannon();
	glPopMatrix();
}

void drawRightBotCannon() {
	glPushMatrix();
	glRotatef(rightCannonAim, 0, 1, 0);
	drawRightCannon();
	//drawProjectile2();
	drawLeftCannon();
	glPopMatrix();
}

void drawHead()
{
	// Set robot material properties per body part. Can have seperate material properties for each part

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
	glutSolidCube(wheelDiameter / 2.0);
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