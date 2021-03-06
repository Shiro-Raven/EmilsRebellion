//  Includes
#include "stdafx.h"
#include <stdio.h>
#include <math.h>
#include <random>
#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <time.h>
#include <stdlib.h>
#include <windows.h>
#include <Mmsystem.h>
//-----------------

enum enemyType { small, big, minion };

#define PI 3.14159265
#define SPACEBAR 32
#define playerY 75
using namespace std;

//  Methods Signatures
void printText(int x, int y, char *string);
void drawHalfCircle(int r);
void drawCircle(int x, int y, float r);
void drawRect(int x, int y, int w, int h, bool isFilled);

void drawStar(int dx, int dy);
void drawPowerUp(float dx, float dy, bool powerOrSpeed);
void drawBullet(float dx, float dy, int index);
void drawObstacle(float dx, float dy);
void drawSpaceship(void);
void drawEnemy(float dx, float dy, enemyType type);
void drawMinion(void);

void firstBackgroundMakerTimer(int value);
void firstBackgroundMoverTimer(int value);
void secondBackgroundMakerTimer(int value);
void secondBackgroundMoverTimer(int value);
void powerUpMakerTimer(int value);
void powerUpTimeout(int value);
void obstacleMakerTimer(int value);
void objectsMoveTimer(int value);
void keyTimer(int value);
void Timer(int value);

void obstaclesCollisions(int value);
void powerUpCollisions(int value);
void bulletsCollisions(int value);
void minionTimer(int value);

void resetGame(void);
void Key(unsigned char key, int x, int y);
void KeyUp(unsigned char key, int x, int y);
void Display(void);
//-----------------

//		Global Variables

// Player
int score = 0;
int playerX = 400;
int playerRotation = 0;

// Background
vector<int> starsXs = {};
vector<int> starsYs = {};

vector<int> planetsXs = {};
vector<int> planetsYs = {};
vector<int> planetsScales = {};

// Bullets
vector<int> bulletsXs = {};
vector<int> bulletsYs = {};

// Obstacles 
vector<int> obstaclesXs = {};
vector<int> obstaclesYs = {};

// PowerUps
vector<int> powerupsXs = {};
vector<int> powerupsYs = {};
vector<bool> powerupsTypes = {};
int speedUpIndex = -1;
int strongerIndex = -1;
bool isStronger = false;
bool isFaster = false;
int strongTimer;
int fastTimer;

// Controls
bool clicked = false;
bool isDead = false;
bool playOnce = false;
bool keyStates[256];
bool isWon = false;
bool anotherOnce = false;

// Bezier and Enemy movement
bool isSmallEnemy = true;
int enemyX = 100;
int enemyY = 300;
float enemyRotation = 0;
int enemyHealth = 100;

// Minion
bool isActive = false;
int minionTimeout;

bool toTheRight = true;
float t = 0;
int p0[2] = { 100, 400 };
int p1[2] = { 400, 600 };
int p2[2] = { 400, 100 };
int p3[2] = { 700, 400 };

int tmpX = 0, tmpY = 600;

bool once = false;

float* bezier(float t, int* p0, int* p1, int* p2, int* p3)
{
	float res[2];

	res[0] = pow((1 - t), 3)*p0[0] +
		3 * t*pow((1 - t), 2)*p1[0] +
		3 * t*t*(1 - t)*p2[0] +
		pow(t, 3)*p3[0];

	res[1] = pow((1 - t), 3)*p0[1] +
		3 * t*pow((1 - t), 2)*p1[1] +
		3 * pow(t, 2)*(1 - t)*p2[1] +
		pow(t, 3)*p3[1];

	float dx = 3 * pow((1 - t), 2)*(p1[0] - p0[0]) +
		6 * t*(1 - t)*(p2[0] - p1[0]) +
		3 * t*t*(p3[0] - p2[0]);
	float dy = 3 * pow((1 - t), 2)*(p1[1] - p0[1]) +
		6 * t*(1 - t)*(p2[1] - p1[1]) +
		3 * t*t*(p3[1] - p2[1]);

	float theta = 180 / PI * atan(dy / dx);

	if (toTheRight) {
		enemyRotation = (theta + 90) + 180;
	}
	else {
		enemyRotation = (theta - 90) + 180;
	}

	return res;
}

void regenerateCurve() {
	int tmp[2] = { p3[0], p3[1] };
	p3[0] = p0[0];
	p3[1] = p0[1];
	p0[0] = tmp[0];
	p0[1] = tmp[1];
	p1[0] = rand() % 600 + 100;
	p2[0] = rand() % 600 + 100;
}

void printText(int x, int y, char *string)
{
	int len, i;

	//set the position of the text in the window using the x and y coordinates
	glRasterPos2f(x, y);

	//get the length of the string to display
	len = (int)strlen(string);

	//loop to display character by character
	for (i = 0; i < len; i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
	}
}

void printStats() {
	// Score 
	char integer_string[32];

	sprintf_s(integer_string, "%d", score);

	char other_string[64] = "Score: ";

	strcat_s(other_string, integer_string);

	// Health Bar
	glColor3f(1, 1, 1);
	drawRect(700, 550, 95, 25, false);
	glColor3f(0, 1, 0);

	float maxHealth = isSmallEnemy ? 100 : 200;

	float newHealth = 95.0 * (enemyHealth / maxHealth);
	drawRect(700, 550, (int)newHealth, 25, true);

	glColor3f(1, 1, 1);
	printText(700, 580, other_string);
}

void fillScreen() {
	for (int i = 0; i < 800; i += 5) {
		starsXs.push_back(i);
		starsYs.push_back(rand() % 600);
	}
}

void transistionRect() {
	glBegin(GL_QUADS);
	glColor3f(0, 0, 0);

	// Transition
	glVertex2f(tmpX, tmpY);
	glVertex2f(tmpX + 800, tmpY);
	glColor3f(0.360, 0.117, 0.501);
	glVertex2f(tmpX + 800, tmpY + 150);
	glVertex2f(tmpX, tmpY + 150);


	// First before clear
	glVertex2f(tmpX, tmpY + 150);
	glVertex2f(tmpX + 800, tmpY + 150);
	glVertex2f(tmpX + 800, tmpY + 600 + 150);
	glVertex2f(tmpX, tmpY + 600 + 150);


	glEnd();
}

void main(int argc, char** argr) {
	glutInit(&argc, argr);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(200, 150);
	glutCreateWindow("The Rebellion of the Emils");
	glutDisplayFunc(Display);
	glutKeyboardFunc(Key);
	glutKeyboardUpFunc(KeyUp);

	fillScreen();

	glutTimerFunc(0, Timer, 0);
	glutTimerFunc(0, firstBackgroundMakerTimer, 0);
	glutTimerFunc(0, firstBackgroundMoverTimer, 0);
	glutTimerFunc(0, keyTimer, 0);
	glutTimerFunc(0, objectsMoveTimer, 1);
	glutTimerFunc(0, bulletsCollisions, 2);
	glutTimerFunc(0, powerUpCollisions, 0);
	glutTimerFunc(0, obstaclesCollisions, 0);
	glutTimerFunc((rand() % 2 + 3) * 1000, obstacleMakerTimer, 0);
	glutTimerFunc((rand() % 1 + 5) * 1000, powerUpMakerTimer, 0);
	glutTimerFunc(10 * 1000, minionTimer, 0);

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glClearColor(0, 0, 0, 0);
	gluOrtho2D(0, 800, 0, 600);

	glutMainLoop();
}

void Key(unsigned char key, int x, int y) {
	clicked = true;
	keyStates[key] = true;

	if (key == SPACEBAR && bulletsXs.size() <= 10) {
		bulletsXs.push_back(playerX);
		bulletsYs.push_back(120);
		if (!PlaySound(TEXT("fire.wav"), NULL, SND_ASYNC | SND_FILENAME)) {
			cout << "Something went wrong" << endl;
		}
	}
	else if (isDead && !isWon && key == 'r') {
		resetGame();
	}

	glutPostRedisplay();
}

void KeyUp(unsigned char key, int x, int y) {
	clicked = false;
	keyStates[key] = false;

	glutPostRedisplay();
}

void resetGame() {
	score = 0;
	playerX = 400;
	playerRotation = 0;

	enemyHealth = 100;
	isSmallEnemy = true;
	isDead = false;

	starsXs = {};
	starsYs = {};

	planetsXs = {};
	planetsYs = {};
	planetsScales = {};

	// Bullets
	bulletsXs = {};
	bulletsYs = {};

	// Obstacles 
	obstaclesXs = {};
	obstaclesYs = {};

	// PowerUps
	powerupsXs = {};
	powerupsYs = {};
	powerupsTypes = {};
	isStronger = false;
	isFaster = false;

	// Bezier and Enemy movement
	enemyX = 100;
	enemyY = 300;
	enemyRotation = 0;

	// Minion
	isActive = false;
	minionTimeout = 0;

	toTheRight = true;
	t = 0;
	p0[0] = 100;
	p0[1] = 400;

	p1[0] = 400;
	p1[1] = 600;

	p2[0] = 400;
	p2[1] = 100;

	p3[0] = 700;
	p3[1] = 400;

	tmpX = 0;
	tmpY = 600;

	once = true;
	playOnce = false;
	anotherOnce = false;

	glutTimerFunc(10 * 1000, minionTimer, 0);

	fillScreen();
}

void Timer(int value) {
	// Move the enemy
	float* p = bezier(t, p0, p1, p2, p3);
	t = t + 0.001f;

	enemyX = (int)p[0];
	enemyY = (int)p[1];

	if (t >= 1) {
		t = 0;
		toTheRight = !toTheRight;
		regenerateCurve();
	}

	// Reset rotation of player
	if (!clicked) {
		if (playerRotation < 0)
			playerRotation++;
		if (playerRotation > 0)
			playerRotation--;
	}

	glutTimerFunc(5, Timer, 1);
}

void firstBackgroundMakerTimer(int value) {
	if (isSmallEnemy) {
		if (starsXs.size() < 10000) {
			starsXs.push_back(rand() % 800);
			starsYs.push_back(600);
		}
		glutTimerFunc(100, firstBackgroundMakerTimer, value);
	}
	else {
		if (tmpY < 400) {
			if (!anotherOnce) {
				anotherOnce = true;
				glutTimerFunc(0, secondBackgroundMakerTimer, 0);
			}
		}
		glutTimerFunc(100, firstBackgroundMakerTimer, value);
	}
}

void firstBackgroundMoverTimer(int value) {
	for (int i = 0; i < starsXs.size(); i++) {
		starsYs[i]--;
		if (starsYs[i] <= 0) {
			starsYs.erase(starsYs.begin() + i);
			starsXs.erase(starsXs.begin() + i);
		}
	}

	if (!isSmallEnemy && tmpY + 150 > 0) {
		tmpY--;
	}

	if (tmpY > 400 && !once) {
		once = true;
		glutTimerFunc(0, secondBackgroundMoverTimer, 0);
	}

	glutPostRedisplay();

	glutTimerFunc(25, firstBackgroundMoverTimer, 0);
}

void secondBackgroundMakerTimer(int value) {
	if (!isSmallEnemy) {
		planetsXs.push_back(rand() % 800);
		planetsYs.push_back(600);
		planetsScales.push_back(rand() % 10);
	}

	glutTimerFunc(1000, secondBackgroundMakerTimer, value);
}

void secondBackgroundMoverTimer(int value) {
	for (int i = 0; i < planetsXs.size(); i++) {
		planetsYs[i]--;
		if (planetsYs[i] <= 0) {
			planetsYs.erase(planetsYs.begin() + i);
			planetsXs.erase(planetsXs.begin() + i);
			planetsScales.erase(planetsScales.begin() + i);
		}
	}

	glutPostRedisplay();

	glutTimerFunc(100, secondBackgroundMoverTimer, 0);
}

void keyTimer(int value) {
	if (keyStates['a'] && playerX > 75) {
		playerX -= 5;
		playerRotation = playerRotation < 15 ? playerRotation + 1 : 15;
	}
	else if (keyStates['d'] && playerX < 725) {
		playerX += 5;
		playerRotation = playerRotation > -15 ? playerRotation - 1 : -15;
	}

	glutPostRedisplay();

	glutTimerFunc(10, keyTimer, 0);
}

void objectsMoveTimer(int value) {
	// Bullets
	for (int i = 0; i < bulletsXs.size(); i++) {
		if (isFaster && i >= speedUpIndex) {
			bulletsYs[i] += 4;
		}
		else {
			bulletsYs[i]++;
		}
		if (bulletsYs[i] >= 600) {
			bulletsXs.erase(bulletsXs.begin() + i);
			bulletsYs.erase(bulletsYs.begin() + i);
			if (speedUpIndex > 0) {
				speedUpIndex--;
			}
			if (strongerIndex > 0) {
				strongerIndex--;
			}
		}
	}

	// Obstacles
	for (int i = 0; i < obstaclesXs.size(); i++) {
		obstaclesYs[i]--;
		if (obstaclesYs[i] <= 0) {
			obstaclesXs.erase(obstaclesXs.begin() + i);
			obstaclesYs.erase(obstaclesYs.begin() + i);
		}
	}

	// The Powerups
	for (int i = 0; i < powerupsXs.size(); i++) {
		powerupsYs[i]--;
		if (powerupsYs[i] <= 0) {
			powerupsXs.erase(powerupsXs.begin() + i);
			powerupsYs.erase(powerupsYs.begin() + i);
			powerupsTypes.erase(powerupsTypes.begin() + i);
		}
	}

	glutTimerFunc(5, objectsMoveTimer, 0);
}

void powerUpCollisions(int value) {
	for (int k = 0; k < powerupsXs.size(); k++) {
		if (powerupsXs[k] <= playerX + 60 && powerupsXs[k] >= playerX - 60 && playerY >= powerupsYs[k]) {
			if (powerupsTypes[k]) {
				isStronger = true;
				strongTimer = 5000;
				strongerIndex = strongerIndex == -1 ? bulletsXs.size() : strongerIndex;
				glutTimerFunc(0, powerUpTimeout, 1);
			}
			else {
				isFaster = true;
				fastTimer = 5000;
				speedUpIndex = speedUpIndex == -1 ? bulletsXs.size() : speedUpIndex;
				glutTimerFunc(0, powerUpTimeout, 0);
			}
			powerupsXs.erase(powerupsXs.begin() + k);
			powerupsYs.erase(powerupsYs.begin() + k);
			powerupsTypes.erase(powerupsTypes.begin() + k);
		}
	}

	glutTimerFunc(2, powerUpCollisions, 0);
}

void obstaclesCollisions(int value) {
	if (!isDead && (isSmallEnemy || tmpY < 0)) {
		for (int j = 0; j < obstaclesXs.size(); j++) {
			if ((abs(playerX - obstaclesXs[j] - 40) <= 60 ||
				abs(playerX - obstaclesXs[j] + 40) <= 60) &&
				obstaclesYs[j] <= playerY + 30 && obstaclesYs[j] >= playerY - 30) {
				obstaclesXs.erase(obstaclesXs.begin() + j);
				obstaclesYs.erase(obstaclesYs.begin() + j);
				if (!isWon) {
					PlaySound(TEXT("playerDies.wav"), NULL, SND_FILENAME);
				}
				isDead = true;
				break;
			}
		}
	}

	glutPostRedisplay();

	glutTimerFunc(2, obstaclesCollisions, 0);
}

void bulletsCollisions(int value) {
	int deltaX, deltaY, difference;
	int m_deltaX, m_deltaY, m_difference;

	bool hitSomething = false;

	for (int i = 0; i < bulletsXs.size(); i++) {
		for (int j = 0; j < obstaclesXs.size(); j++) {
			if (bulletsXs[i] <= obstaclesXs[j] + 40 && bulletsXs[i] >= obstaclesXs[j] - 40
				&& bulletsYs[i] <= obstaclesYs[j] + 10 && bulletsYs[i] >= obstaclesYs[j] - 10) {
				bulletsXs.erase(bulletsXs.begin() + i);
				bulletsYs.erase(bulletsYs.begin() + i);
				obstaclesXs.erase(obstaclesXs.begin() + j);
				obstaclesYs.erase(obstaclesYs.begin() + j);
				if (!PlaySound(TEXT("explosion.wav"), NULL, SND_ASYNC | SND_FILENAME)) {
					cout << "Something went wrong" << endl;
				}
				goto SKIP;
			}
		}

		int offset = isSmallEnemy ? 90 : 115;

		if (isActive) {
			m_deltaX = abs(enemyX - bulletsXs[i]);
			m_deltaY = abs(enemyY - offset - (bulletsYs[i] + 10));

			m_difference = sqrt(pow(m_deltaX, 2) + pow(m_deltaY, 2));
			if (m_difference <= 30) {
				bulletsXs.erase(bulletsXs.begin() + i);
				bulletsYs.erase(bulletsYs.begin() + i);
				if (!PlaySound(TEXT("boing.wav"), NULL, SND_ASYNC | SND_FILENAME)) {
					cout << "Something went wrong" << endl;
				}
				goto SKIP;
			}
		}

		if (enemyHealth > 0) {
			deltaX = abs(enemyX - bulletsXs[i]);
			deltaY = abs(enemyY - (bulletsYs[i] + 10));

			difference = sqrt(pow(deltaX, 2) + pow(deltaY, 2));

			int enemySize = isSmallEnemy ? 50 : tmpY < 0 ? 75 : 0;

			if (difference <= enemySize) {
				bulletsXs.erase(bulletsXs.begin() + i);
				bulletsYs.erase(bulletsYs.begin() + i);
				if (isStronger) {
					enemyHealth -= 10;
				}
				else {
					enemyHealth -= 5;
				}
				if (!PlaySound(TEXT("explosion.wav"), NULL, SND_ASYNC | SND_FILENAME)) {
					cout << "Something went wrong" << endl;
				}
				if (enemyHealth <= 0) {
					if (isSmallEnemy) {
						isSmallEnemy = false;
						isActive = false;
						enemyHealth = 200;
					}
					else {
						isWon = true;
						isActive = false;
						PlaySound(TEXT("win.wav"), NULL, SND_ASYNC | SND_FILENAME);
						cout << "You won" << endl;
					}
				}
				score++;
			}
		}

	SKIP: continue;
	}

DONE: glutPostRedisplay();

	glutTimerFunc(2, bulletsCollisions, 0);
}

void obstacleMakerTimer(int value) {
	int enemySize, offset;

	enemySize = isSmallEnemy ? 50 : 75;
	offset = isSmallEnemy ? 110 : 135;

	if (value == 0) {
		if (!isDead && isSmallEnemy) {
			obstaclesXs.push_back(enemyX);
			obstaclesYs.push_back(enemyY - enemySize);
		}
		glutTimerFunc(rand() % 1 + 3 * 1000, obstacleMakerTimer, 0);
	}
	else if (isActive) {
		obstaclesXs.push_back(enemyX);
		obstaclesYs.push_back(enemyY - offset);
		glutTimerFunc(rand() % 2 + 2 * 1000, obstacleMakerTimer, 1);
	}
}

void powerUpMakerTimer(int value) {
	powerupsXs.push_back(rand() % 500 + 100);
	powerupsYs.push_back(600);
	bool type = value == 0;
	powerupsTypes.push_back(type);

	glutTimerFunc((rand() % 1 + 5) * 1000, powerUpMakerTimer, rand() % 2);
}

void powerUpTimeout(int value) {
	if (value == 1) {
		strongTimer--;
		if (strongTimer <= 0) {
			isStronger = false;
			strongerIndex = -1;
		}
		else {
			glutTimerFunc(1, powerUpTimeout, value);
		}
	}
	else {
		fastTimer--;
		if (fastTimer <= 0) {
			isFaster = false;
			speedUpIndex = -1;
		}
		else {
			glutTimerFunc(1, powerUpTimeout, value);
		}
	}
}

void minionTimer(int value) {
	if (!isActive && !isDead && (isSmallEnemy || tmpY <= 0)) {
		isActive = true;
		glutTimerFunc(rand() % 2 + 2 * 1000, obstacleMakerTimer, 1);
		glutTimerFunc(1, minionTimer, 10000);
	}
	else if (!isDead) {
		int remaining = value - 1;
		if (remaining <= 0) {
			isActive = false;
			glutTimerFunc(10 * 1000, minionTimer, 0);
		}
		else {
			glutTimerFunc(1, minionTimer, remaining);
		}
	}
}

void Display() {
	glClear(GL_COLOR_BUFFER_BIT);

	if (!isSmallEnemy) {
		if (tmpY + 150 <= 0) {
			glClearColor(0.360, 0.117, 0.501, 1);
		}
		else {
			transistionRect();
		}
	}
	else {
		glClearColor(0, 0, 0, 1);
	}

	for (int i = 0; i < starsXs.size(); i++) {
		drawStar(starsXs[i], starsYs[i]);
	}

	glPushMatrix();
	glColor3f(0.184, 0.117, 0.501);
	for (int i = 0; i < planetsXs.size(); i++) {
		drawCircle(planetsXs[i], planetsYs[i], 3 * planetsScales[i]);
	}
	glPopMatrix();

	if (!isDead && !isWon) {
		printStats();
		for (int i = 0; i < bulletsXs.size(); i++) {
			drawBullet(bulletsXs[i], bulletsYs[i], i);
		}

		for (int i = 0; i < obstaclesXs.size(); i++) {
			drawObstacle(obstaclesXs[i], obstaclesYs[i]);
		}


		for (int i = 0; i < powerupsXs.size(); i++) {
			drawPowerUp(powerupsXs[i], powerupsYs[i], powerupsTypes[i]);
		}

		drawSpaceship();

		if (isSmallEnemy) {
			drawEnemy(enemyX, enemyY, small);
		}
		else if (tmpY <= 0) {
			drawEnemy(enemyX, enemyY, big);
		}

		if (isActive) {
			drawMinion();
		}
	}
	else {
		char integer_string[32];
		sprintf_s(integer_string, "%d", score);
		char scoreText[64] = "Score: ";
		strcat_s(scoreText, integer_string);

		if (isWon) {
			playOnce = true;
			glColor3f(0, 0, 0);
			printText(300, 300, "You Won!");
			printText(400, 250, scoreText);
		}
		else if (isDead) {
			glColor3f(1, 1, 1);
			printText(200, 300, "You are dead. :( \n Press R to restart the game.");

			printText(400, 250, scoreText);
		}
	}


	glFlush();

	if (!playOnce && isDead) {
		playOnce = true;
		PlaySound(TEXT("lose.wav"), NULL, SND_FILENAME);
	}
}

// Drawing methods

void drawCircle(int x, int y, float r) {
	glPushMatrix();
	glTranslatef(x, y, 0);
	GLUquadric *quadObj = gluNewQuadric();
	gluDisk(quadObj, 0, r, 50, 50);
	glPopMatrix();
}

void drawRect(int x, int y, int w, int h, bool isFilled) {
	if (isFilled) {
		glBegin(GL_POLYGON);
	}
	else {
		glBegin(GL_LINE_LOOP);
	}
	glVertex2f(x, y);
	glVertex2f(x + w, y);
	glVertex2f(x + w, y + h);
	glVertex2f(x, y + h);
	glEnd();
}

void drawHalfCircle(int r) {
	float radius = r;
	float twoPI = 2 * PI;

	glBegin(GL_TRIANGLE_FAN);
	for (float i = 0.0; i <= twoPI / 2; i += 0.001)
		glVertex2f((sin(i)*radius), (cos(i)*radius));

	glEnd();
}

void drawStar(int dx, int dy) {
	glPushMatrix();
	glColor3f(1, 1, 1);
	drawRect(dx, dy, 5, 5, true);
	glPopMatrix();
}

void drawSpaceship() {
	glPushMatrix();
	glTranslatef(playerX, playerY, 0);
	glRotatef(playerRotation, 0, 0, 1);

	glPushMatrix();
	glScalef(0.7, 0.7, 1);

	// Wings
	glPushMatrix();
	glColor3f(1, 0, 0);

	// Front
	glPushMatrix();
	glTranslatef(0, 25, 0);
	glBegin(GL_TRIANGLES);
	glVertex3f(-20, 0, 0);
	glVertex3f(20, 0, 0);
	glVertex3f(0, 20, 0);
	glEnd();
	glPopMatrix();

	// Side #1
	glPushMatrix();
	glTranslatef(45, 0, 0);
	glBegin(GL_TRIANGLES);
	glVertex3f(0, -20, 0);
	glVertex3f(50, -20, 0);
	glVertex3f(0, 20, 0);
	glEnd();
	glPopMatrix();

	// Side #2
	glPushMatrix();
	glTranslatef(-45, 0, 0);
	glBegin(GL_TRIANGLES);
	glVertex3f(0, -20, 0);
	glVertex3f(-50, -20, 0);
	glVertex3f(0, 20, 0);
	glEnd();
	glPopMatrix();

	glPopMatrix();

	// Exhausts
	glPushMatrix();
	glColor3f(0.811, 0.811, 0.811);

	glPushMatrix();
	glTranslatef(15, -50, 0);
	drawRect(0, 0, 20, 30, true);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-35, -50, 0);
	drawRect(0, 0, 20, 30, true);
	glPopMatrix();

	glPopMatrix();

	// The hull
	glPushMatrix();
	glColor3d(0, 0, 1);
	glScalef(2, 1, 1);
	drawCircle(0, 0, 30);
	glPopMatrix();

	// The windows
	glPushMatrix();
	glColor3f(0.945, 0.980, 0.058);
	// Window one
	glPushMatrix();
	glTranslatef(30, 0, 0);
	drawCircle(0, 0, 10);
	glPopMatrix();
	// Window two
	glPushMatrix();
	glTranslatef(-30, 0, 0);
	drawCircle(0, 0, 10);
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();
}

void drawEnemy(float dx, float dy, enemyType type) {
	glPushMatrix();
	glTranslatef(dx, dy, 0);
	switch (type) {
	case small: glRotatef(enemyRotation, 0, 0, 1); break;
	case big: glRotatef(enemyRotation, 0, 0, 1); glScalef(1.5, 1.5, 1); break;
	case minion: glScalef(0.6, 0.6, 1); break;
	}

	// Horns
	if (type == big) {
		glBegin(GL_TRIANGLES);
		glColor3f(1, 0, 0);
		glVertex3f(0, 0, 0);
		glVertex3f(20, 0, 0);
		glVertex3f(60, 60, 0);

		glVertex3f(0, 0, 0);
		glVertex3f(-20, 0, 0);
		glVertex3f(-60, 60, 0);
		glEnd();
	}

	// Right Arm
	glPushMatrix();
	switch (type) {
	case small: glColor3f(0.666, 0.145, 0.733); break;
	case big: glColor3f(0, 0, 0); break;
	case minion: glColor3f(0.196, 0.772, 0.984); break;
	}
	glRotatef(-120, 0, 0, 1);
	drawRect(-5, 0, 10, 100, true);
	drawRect(5, 90, 50, 10, true);
	glBegin(GL_TRIANGLES);
	glColor3f(1, 0, 0);
	glVertex3f(55, 90, 0);
	glVertex3f(55, 100, 0);
	glVertex3f(65, 95, 0);
	glEnd();
	glPopMatrix();

	// Left Arm
	glPushMatrix();
	switch (type) {
	case small: glColor3f(0.666, 0.145, 0.733); break;
	case big: glColor3f(0, 0, 0); break;
	case minion: glColor3f(0.196, 0.772, 0.984); break;
	}
	glScalef(-1, 1, 1);
	glRotatef(-120, 0, 0, 1);
	drawRect(-5, 0, 10, 100, true);
	drawRect(5, 90, 50, 10, true);
	glBegin(GL_TRIANGLES);
	glColor3f(1, 0, 0);
	glVertex3f(55, 90, 0);
	glVertex3f(55, 100, 0);
	glVertex3f(65, 95, 0);
	glEnd();
	glPopMatrix();

	switch (type) {
	case small: glColor3f(0.666, 0.145, 0.733); break;
	case big: glColor3f(0, 0, 0); break;
	case minion: glColor3f(1, 1, 0); break;
	}
	// Head
	drawCircle(0, 0, 50);

	// Eyes
	glPushMatrix();
	switch (type) {
	case big: glColor3f(1, 0, 0); break;
	default: glColor3f(0.176, 0.733, 0.145);
	}

	glPushMatrix();
	glTranslatef(20, 20, 0);
	drawCircle(0, 0, 15);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-20, 20, 0);
	drawCircle(0, 0, 15);
	glPopMatrix();

	// Mouth
	glPushMatrix();
	glRotatef(-90, 0, 0, 1);
	switch (type) {
	case big: glColor3f(1, 0, 0); break;
	default: glColor3f(0.176, 0.733, 0.145);
	}
	drawHalfCircle(40);
	glPopMatrix();
	glBegin(GL_LINES);
	glVertex3f(-40, 0, 0);
	glVertex3f(40, 0, 0);
	glPushMatrix();
	glColor3f(0, 0, 0);
	glVertex3f(-20, 0, 0);
	glVertex3f(-20, -35, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(0, -40, 0);
	glVertex3f(20, 0, 0);
	glVertex3f(20, -35, 0);
	glVertex3f(-36, -18, 0);
	glVertex3f(36, -18, 0);
	glPopMatrix();
	glEnd();

	glPopMatrix();

	glPopMatrix();
}

void drawMinion() {
	glPushMatrix();

	int offset = isSmallEnemy ? 90 : 115;
	drawEnemy(enemyX, enemyY - offset, minion);

	glPopMatrix();
}

void drawBullet(float dx, float dy, int index) {
	glPushMatrix();
	glTranslatef(dx, dy, 0);
	bool stronger = isStronger;
	if (stronger && index >= strongerIndex) {
		glPushMatrix();
		glScalef(1.5, 1.5, 1);
	}

	glColor3f(0.549, 0.549, 0.549);
	drawRect(-5, -10, 10, 20, true);

	glColor3f(1, 0, 0);
	glBegin(GL_TRIANGLES);
	glVertex3f(-10, 10, 0);
	glVertex3f(10, 10, 0);
	glVertex3f(0, 20, 0);
	glEnd();

	glColor3f(1, 1, 0);
	glBegin(GL_TRIANGLES);
	glVertex3f(-5, -10, 0);
	glVertex3f(-10, -10, 0);
	glVertex3f(-5, -5, 0);

	glVertex3f(5, -10, 0);
	glVertex3f(10, -10, 0);
	glVertex3f(5, -5, 0);
	glEnd();

	if (stronger && index >= strongerIndex) {
		glPopMatrix();
	}

	glPopMatrix();
}

void drawObstacle(float dx, float dy) {
	glPushMatrix();
	glTranslatef(dx, dy, 0);

	glColor3f(0.5, 0.5, 0.5);
	drawRect(-40, -10, 80, 20, true);

	glBegin(GL_TRIANGLES);
	glColor3f(0.666, 0.145, 0.733);
	glVertex3f(-40, -10, 0);
	glVertex3f(-40, 10, 0);
	glVertex3f(-50, 20, 0);

	glVertex3f(40, -10, 0);
	glVertex3f(40, 10, 0);
	glVertex3f(50, 20, 0);
	glEnd();

	glPopMatrix();
}

void drawPowerUp(float dx, float dy, bool powerOrSpeed) {
	if (powerOrSpeed) {
		// Stronger bullets 
		glPushMatrix();
		glTranslatef(dx, dy, 0);
		glColor3f(0.196, 0.521, 0.984);
		drawCircle(0, 0, 15);

		glColor3f(0.984, 0.380, 0.196);
		drawRect(-12.5, -2.5, 25, 5, true);
		drawRect(-2.5, -12.5, 5, 25, true);

		glPopMatrix();
	}
	else {
		// Faster bullets
		glPushMatrix();
		glTranslatef(dx, dy, 0);
		glColor3f(0.196, 0.521, 0.984);
		drawCircle(0, 0, 15);

		glColor3f(1, 1, 0);
		glPushMatrix();
		GLfloat m[16] = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.5, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};
		glMultMatrixf(m);
		drawRect(-8, 0, 10, 10, true);
		glPopMatrix();

		glBegin(GL_TRIANGLES);
		glVertex3f(-7, -10, 0);
		glVertex3f(-2, 0, 0);
		glVertex3f(10, 0, 0);
		glEnd();

		glPopMatrix();
	}
}