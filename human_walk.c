/* 
 * コンピュータグラフィックス演習
 * レポート3（自由制作課題）
 *
 * 学籍番号: ******
 * 氏名: khengyeww
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <GL/glut.h>
#include <GL/gl.h>

#include "./myShape.h"

#define KEY_ESC 27
#define SPACEBAR 32

GLint ourWidth = 960;   //  ウィンドウの幅
GLint ourHeight = 640;  //  ウィンドウの高さ

GLfloat angle = 0.0f;
int miss = 0; // 答えを間違った回数
int clear = 0; // 正しく回答できた回数
int seq = 0; // 画面の順序
int que = 1; // 質問の番号
int level = 1; // 質問の難易度
int timer = 0; // 時間の計算 (timer = 10 の時は1秒)
int walkTimer = 0; // 歩く時間の計算 (timer = 10 の時は1秒)
int show_Score = 0; // 最後の画面表示に使う (keep track)
float progress = 0.0;

GLfloat CamX = 0.0f;
GLfloat movex = 0.0f;
GLfloat timerMovex = 0.0f;
unsigned char winCond = GL_FALSE;
unsigned char clearStage = GL_FALSE;
unsigned char timerFlag = GL_FALSE;
unsigned char loseFlag = GL_FALSE;
unsigned char goalFlag = GL_FALSE;
unsigned char stopFlag = GL_FALSE; // タイマーバーを停止する
//unsigned char readyFlag = GL_TRUE; // 次の質問の表示

#define imageWidth 128
#define imageHeight 128
#define NumOfTextureFiles 10

static const char *images[] = {
  "t16cs001_SadSmiley.ppm",
  "t16cs001_Grassland.ppm",
  "t16cs001_win1_tensai.ppm",
  "t16cs001_woah.ppm",
  "t16cs001_jail.ppm",
  "t16cs001_win2_maamaa.ppm",
  "t16cs001_memedog.ppm",
  "t16cs001_win3_luck.ppm",
  "t16cs001_???man.ppm",
  "t16cs001_sponge.ppm"
};

GLubyte image[imageHeight][imageWidth][3];
GLint texID[NumOfTextureFiles];

char* get_word(void);
void LoadASCIIPPMImage(const char* filename);

static FILE* texturefile;
static char readbuf[256];
static char word[256];
static char* ptr = readbuf;
static char* ptr2 = word;
/*  ---------------------------------------  */

//  視線位置・方向を保持するための構造体 
struct EyePint
{
	GLfloat x;		//  視点のx座標
	GLfloat y;		//  視点のy座標
	GLfloat z;		//  視点のz座標
	GLfloat rotx;	//  視点のx座標軸回転の角度
	GLfloat roty;	//  視点のy座標軸回転の角度
	GLfloat rotz;	//  視点のz座標軸回転の角度
} ourEyePoint; 

//  光源の設定
void InitLight()
{
	GLfloat ambient[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat diffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat specular[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	GLfloat position[4] = { 100.0f, 10.0f, 1000.0f, 0.0f };

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

}

// 大域変数をリセットする関数
void Reset(){
  miss = 0;
  clear = 0;
  que = 1;
  movex = 0;
  show_Score = 0;
  loseFlag = GL_FALSE;
  clearStage = GL_FALSE;
  goalFlag = GL_FALSE;
}

// タイマー用の変数をリセットする関数
void TimerReset(){
  timerMovex = 0.0;
  timer = 0;
  progress = 0.0;
  stopFlag = GL_FALSE;
}

// メイン画面の Start
void startBox()
{
  GLfloat material_specular[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
  GLfloat material_diffuse[4] = { 0.3f, 0.1f, 0.0f, 1.0f };
  GLfloat material_ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

  glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
  glMaterialf(GL_FRONT, GL_SHININESS, 128.0);

  glPushMatrix();
  glTranslatef(0.0, 3.0, -6.0);
  glRotatef(15.0, -5.0, 0.0, 0.0);
  glScalef(17.0, 8.0, 0.1);
  glutSolidCube(4.5);
  glPopMatrix();

  char a[]="START";
  glPushMatrix();
  glTranslatef(-13.0, 0.0, 8.0);
  glRotatef(15.0, -5.0, 0.0, 0.0);
  glScalef(0.05, 0.05, 0.05);
  int i;
  for(i=0;i<5;i++)
    glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, a[i]);
  glPopMatrix();
}

// メイン画面の quit
void quitBox(){
  GLfloat material_specular[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
  GLfloat material_diffuse[4] = { 0.1f, 0.0f, 0.0f, 1.0f };
  GLfloat material_ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

  glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
  glMaterialf(GL_FRONT, GL_SHININESS, 128.0);

  glPushMatrix();
  glTranslatef(0.0, -15.0, 6.0);
  glRotatef(15.0, -5.0, 0.0, 0.0);
  glScalef(20.0, 7.0, 0.1);
  glutSolidCube(1.0);
  glPopMatrix();

  char a[]="QUIT?";
  glPushMatrix();
  glTranslatef(-6.5, -13.0, 8.0);
  glRotatef(15.0, -5.0, 0.0, 0.0);
  glScalef(0.025, 0.025, 0.025);
  int i;
  for(i=0;i<5;i++)
    glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, a[i]);
  glPopMatrix();

    glPushMatrix();
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_TEXTURE_2D );
    glEnable( GL_LIGHTING );
    glRotatef(15.0, -5.0, 0.0, 0.0);
    glBindTexture(GL_TEXTURE_2D, texID[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(1.0,0.0); glVertex3f(10.7, -20.9, 1.0);
    glTexCoord2f(0.0,0.0); glVertex3f(17.7, -20.9, 1.0);
    glTexCoord2f(0.0,1.0); glVertex3f(17.7, -13.4, 1.0);
    glTexCoord2f(1.0,1.0); glVertex3f(10.7, -13.4, 1.0);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

// 人間モデルを描く
void DrawHuman(void)
{
  glShadeModel(GL_FLAT);

  GLfloat material_specular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
  GLfloat material_diffuse[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
  GLfloat material_ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

  glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
  glMaterialf(GL_FRONT, GL_SHININESS, 128.0);

	//  胴体 
	glPushMatrix();
	glTranslatef(-10+movex, 2.0, 5.0);
	mySolidCylinder(1.2, 3.0, 20);
	glPopMatrix();

	//  頭部 
	glPushMatrix();
	glTranslatef(-10.0+movex, 4.3, 5.0);
	glutSolidSphere(1.2, 20, 20);

	//  左腕
	glPushMatrix();
	glRotatef(90.0, 0.0, 1.0, 0.0);
	  //  上腕 
	glTranslatef(1.5, -1.2, 0.0);
	  if(winCond)
	    glRotatef(30.0*cos(angle), -1.0, 0.0, 0.0);

	  glTranslatef(0.0, -0.75, 0.0);
	  glPushMatrix();
	    glScalef(0.5, 2.2, 0.5);
	    glutSolidCube(1.0);
	  glPopMatrix();
	  glTranslatef(0.0, 0.75, 0.0);

          //  下腕 
	  glPushMatrix();
	    //glTranslatef(1.0, 1.0, 0.0);
	    glTranslatef(0.0, -1.6, 0.0);
	    if(winCond)
	      glRotatef(30.0*(1.0 + cos(angle)), -1.0, 0.0, 0.0);

	    glPushMatrix();
	      glTranslatef(0.0, -0.8, 0.0);
	      glScalef(0.5, 1.0, 0.5);
	      glutSolidCube(1.0);
	      //glTranslatef(0.0, 0.3, 0.0);
	    glPopMatrix();
	  glPopMatrix();
	glPopMatrix();

	//  右腕
	glPushMatrix();
	glRotatef(90.0, 0.0, 1.0, 0.0);
	  //  上腕 
	  glTranslatef(-1.5, -1.2, 0.0);
	  if(winCond)
	    glRotatef(30.0*cos(angle), 1.0, 0.0, 0.0);

	  glTranslatef(0.0, -0.75, 0.0);
	  glPushMatrix();
	    glScalef(0.5, 2.2, 0.5);
	    glutSolidCube(1.0);
	  glPopMatrix();
	  glTranslatef(0.0, 0.75, 0.0);

	  //  下腕 
	  glPushMatrix();
	    //glTranslatef(-1.0, 1.0, 0.0);
	    glTranslatef(0.0, -1.6, 0.0);
	    if(winCond)
	      glRotatef(30.0*(-1.0 + cos(angle)), 1.0, 0.0, 0.0);

	    glPushMatrix();
	      glTranslatef(0.0, -0.8, 0.0);
	      glScalef(0.5, 1.0, 0.5);
	      glutSolidCube(1.0);
	      //glTranslatef(0.0, 0.3, 0.0);
	    glPopMatrix();
	  glPopMatrix();
	glPopMatrix();

	//  左足
	glPushMatrix();
	glRotatef(90.0, 0.0, 1.0, 0.0);
	  //  上足
	  glTranslatef(0.5, -4.05, 0.0);

	  if(winCond)
	    glRotatef(20.0*cos(angle), 1.0, 0.0, 0.0);
	  glTranslatef(0.0, -0.75, 0.0);
	  glPushMatrix();
	    //glScalef(1.0, 5.0, 1.0);
	    mySolidCylinder(0.3, 2.1, 20);
	  glPopMatrix();
	  glTranslatef(0.0, 0.75, 0.0);

	  //  下足 
	  glPushMatrix();
	    //glTranslatef(0.35, -1.0, 0.0);
	    glTranslatef(0.0, -1.7, 0.0);
	    if(winCond)
	      glRotatef(20.0*(1.0 + cos(angle)), 1.0, 0.0, 0.0);
	    glPushMatrix();
	      glTranslatef(0.0, -0.8, 0.0);
	      //glScalef(1.0, 2.0, 1.0);
	      mySolidCylinder(0.3, 1.6, 20);
	      glTranslatef(0.0, 0.3, 0.0);
	    glPopMatrix();
	    //
	    glPushMatrix();
	      //glTranslatef(0.35, -3.2, 0.23);
	      glTranslatef(0.0, -1.6, 0.23);
	      glScalef(0.6, 0.2, 1.6);
	      glutSolidCube(1.0);
	      //glTranslatef(0.0, 0.3, 0.0);
	    glPopMatrix();
	  glPopMatrix();
	glPopMatrix();

	//  右足
	glPushMatrix();
	glRotatef(90.0, 0.0, 1.0, 0.0);
	  //  上足
	  glTranslatef(-0.5, -4.05, 0.0);

	  if(winCond)
	    glRotatef(20.0*cos(angle), -1.0, 0.0, 0.0);
	  glTranslatef(0.0, -0.75, 0.0);
	  glPushMatrix();
	    //glScalef(1.0, 5.0, 1.0);
	    mySolidCylinder(0.3, 2.1, 20);
	  glPopMatrix();
	  glTranslatef(0.0, 0.75, 0.0);

	  //  下足 
	  glPushMatrix();
	    //glTranslatef(-0.35, -1.0, 0.0);
	    glTranslatef(0.0, -1.7, 0.0);
	    if(winCond)
	      glRotatef(20.0*(-1.0 + cos(angle)), -1.0, 0.0, 0.0);
	    glPushMatrix();
	      glTranslatef(0.0, -0.8, 0.0);
	      //glScalef(1.0, 2.0, 1.0);
	      mySolidCylinder(0.3, 1.6, 20);
	      glTranslatef(0.0, 0.3, 0.0);
	    glPopMatrix();
	    //
	    glPushMatrix();
	      //glTranslatef(-0.35, -3.2, 0.23);
	      glTranslatef(0.0, -1.6, 0.5);
	      glScalef(0.6, 0.2, 1.6);
	      glutSolidCube(1.0);
	      //glTranslatef(0.0, 0.3, 0.0);
	    glPopMatrix();
	  glPopMatrix();
	glPopMatrix();
	glPopMatrix();
}

// ミス1 を表示する板
void mistakeBox1(){
  GLfloat diffuse_r = 0.0f;
  GLfloat diffuse_g = 1.0f;

  if(miss > 1){
    diffuse_r = 1.0f;
    diffuse_g = 0.0f;
  }

  GLfloat material_specular[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
  GLfloat material_diffuse[4] = { diffuse_r, diffuse_g, 0.0f, 1.0f };
  GLfloat material_ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

  glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
  glMaterialf(GL_FRONT, GL_SHININESS, 128.0);

  glPushMatrix();
  glTranslatef(9.5 + movex, 8.5, 10);
  glScalef(1.0, 1.0, 0.1);
  glutSolidCube(0.3);
  glPopMatrix();
}

// ミス2 を表示する板
void mistakeBox2(){
  GLfloat diffuse_r = 0.0f;
  GLfloat diffuse_g = 1.0f;

  if(miss > 0){
    diffuse_r = 1.0f;
    diffuse_g = 0.0f;
  }

  GLfloat material_specular[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
  GLfloat material_diffuse[4] = { diffuse_r, diffuse_g, 0.0f, 1.0f };
  GLfloat material_ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

  glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
  glMaterialf(GL_FRONT, GL_SHININESS, 128.0);

  glPushMatrix();
  glTranslatef(10+movex, 8.5, 10);
  glScalef(1.0, 1.0, 0.1);
  glutSolidCube(0.3);
  glPopMatrix();
}

// 質問表示の板
void questionBox(){
  GLfloat material_specular[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
  GLfloat material_diffuse[4] = { 1.0f, 0.5f, 1.0f, 1.0f };
  GLfloat material_ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

  glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
  glMaterialf(GL_FRONT, GL_SHININESS, 128.0);

  glPushMatrix();
  glTranslatef(-4.0 + movex, 0.0, 13.5);
  glRotatef(20.0, -5.0, 0.0, 0.0);
  glScalef(7.0, 3.0, 0.1);
  glutSolidCube(0.3);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0 + movex, 0.0, 13.5);
  glRotatef(20.0, -5.0, 0.0, 0.0);
  glScalef(7.0, 3.0, 0.1);
  glutSolidCube(0.3);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(4.0 + movex, 0.0, 13.5);
  glRotatef(20.0, -5.0, 0.0, 0.0);
  glScalef(7.0, 3.0, 0.1);
  glutSolidCube(0.3);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(-3.5 + movex, 8.0, 10.0);
  glRotatef(13.0, -5.0, 0.0, 0.0);
  glScalef(14.2, 1.5, 0.0);
  glutSolidCube(1.0);
  glPopMatrix();
}

// 答えの選択の表示
void selectionAns(){

  int omg;

  if(level == 1){
    char aq[] = "3-9+4-7+5-2+8-1 = ?";
    char bq[] = "14 + 8 x (2+5) / 2 = ?";
    char cq[] = "2^6 - 6(10/2) = ?";

    char a[] = "010203";
    char b[] = "357242";
    char c[] = "123456";

    if(que == 1 && !winCond){
      glPushMatrix();
      glTranslatef(-9.1 + movex, 7.3, 10.5);
      glRotatef(12.0, -5.0, 0.0, 0.0);
      glColor3f(1.0, 0.0, 1.0);
      glScalef(0.006, 0.006, 0.006);
      for(omg=0;omg<19;omg++)
	glutStrokeCharacter(GLUT_STROKE_ROMAN, aq[omg]);
      glPopMatrix();

      glPushMatrix();
      glTranslatef(-4.1 + movex, 0.0, 13.8);
      glRotatef(20.0, -5.0, 0.0, 0.0);
      glColor3f(1.0, 1.0, 0.0);
      glScalef(0.007, 0.007, 0.007);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, a[0]);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, a[1]);
      glPopMatrix();

      glPushMatrix();
      glTranslatef(-0.5 + movex, 0.0, 13.8);
      glRotatef(20.0, -5.0, 0.0, 0.0);
      glColor3f(1.0, 1.0, 0.0);
      glScalef(0.007, 0.007, 0.007);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, a[2]);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, a[3]);
      glPopMatrix();

      glPushMatrix();
      glTranslatef(3.0 + movex, 0.0, 13.8);
      glRotatef(20.0, -5.0, 0.0, 0.0);
      glColor3f(1.0, 1.0, 0.0);
      glScalef(0.007, 0.007, 0.007);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, a[4]);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, a[5]);
      glPopMatrix();
    }
    if(que == 2 && !winCond){
      glPushMatrix();
      glTranslatef(-9.1 + movex, 7.3, 10.5);
      glRotatef(12.0, -5.0, 0.0, 0.0);
      glColor3f(1.0, 0.0, 1.0);
      glScalef(0.006, 0.006, 0.006);
      for(omg=0;omg<22;omg++)
	glutStrokeCharacter(GLUT_STROKE_ROMAN, bq[omg]);
      glPopMatrix();

      glPushMatrix();
      glTranslatef(-4.1 + movex, 0.0, 13.8);
      glRotatef(20.0, -5.0, 0.0, 0.0);
      glColor3f(1.0, 1.0, 0.0);
      glScalef(0.007, 0.007, 0.007);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, b[0]);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, b[1]);
      glPopMatrix();

      glPushMatrix();
      glTranslatef(-0.5 + movex, 0.0, 13.8);
      glRotatef(20.0, -5.0, 0.0, 0.0);
      glColor3f(1.0, 1.0, 0.0);
      glScalef(0.007, 0.007, 0.007);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, b[2]);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, b[3]);
      glPopMatrix();

      glPushMatrix();
      glTranslatef(3.0 + movex, 0.0, 13.8);
      glRotatef(20.0, -5.0, 0.0, 0.0);
      glColor3f(1.0, 1.0, 0.0);
      glScalef(0.007, 0.007, 0.007);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, b[4]);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, b[5]);
      glPopMatrix();
    }
    if(que == 3 && !winCond){
      glPushMatrix();
      glTranslatef(-9.1 + movex, 7.3, 10.5);
      glRotatef(12.0, -5.0, 0.0, 0.0);
      glColor3f(1.0, 0.0, 1.0);
      glScalef(0.006, 0.006, 0.006);
      for(omg=0;omg<17;omg++)
	glutStrokeCharacter(GLUT_STROKE_ROMAN, cq[omg]);
      glPopMatrix();

      glPushMatrix();
      glTranslatef(-4.1 + movex, 0.0, 13.8);
      glRotatef(20.0, -5.0, 0.0, 0.0);
      glColor3f(1.0, 1.0, 0.0);
      glScalef(0.007, 0.007, 0.007);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, c[0]);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, c[1]);
      glPopMatrix();

      glPushMatrix();
      glTranslatef(-0.5 + movex, 0.0, 13.8);
      glRotatef(20.0, -5.0, 0.0, 0.0);
      glColor3f(1.0, 1.0, 0.0);
      glScalef(0.007, 0.007, 0.007);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, c[2]);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, c[3]);
      glPopMatrix();

      glPushMatrix();
      glTranslatef(3.0 + movex, 0.0, 13.8);
      glRotatef(20.0, -5.0, 0.0, 0.0);
      glColor3f(1.0, 1.0, 0.0);
      glScalef(0.007, 0.007, 0.007);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, c[4]);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, c[5]);
      glPopMatrix();
    }
  }
  if(level == 2){
    char aq[] = "30 - 12/3x2 = ?";
    char bq[] = "|4 - 8(3-12)| - |5-11| = ?";
    char cq[] = "11 + sqrt(-4 + 6x4/3) = ?";

    char a[] = "122228";
    char b[] = "708262";
    char c[] = "171513";

    if(que == 1 && !winCond){
      glPushMatrix();
      glTranslatef(-9.1 + movex, 7.3, 10.5);
      glRotatef(12.0, -5.0, 0.0, 0.0);
      glColor3f(1.0, 0.0, 1.0);
      glScalef(0.006, 0.006, 0.006);
      for(omg=0;omg<15;omg++)
	glutStrokeCharacter(GLUT_STROKE_ROMAN, aq[omg]);
      glPopMatrix();

      glPushMatrix();
      glTranslatef(-4.1 + movex, 0.0, 13.8);
      glRotatef(20.0, -5.0, 0.0, 0.0);
      glColor3f(1.0, 1.0, 0.0);
      glScalef(0.007, 0.007, 0.007);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, a[0]);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, a[1]);
      glPopMatrix();

      glPushMatrix();
      glTranslatef(-0.5 + movex, 0.0, 13.8);
      glRotatef(20.0, -5.0, 0.0, 0.0);
      glColor3f(1.0, 1.0, 0.0);
      glScalef(0.007, 0.007, 0.007);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, a[2]);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, a[3]);
      glPopMatrix();

      glPushMatrix();
      glTranslatef(3.0 + movex, 0.0, 13.8);
      glRotatef(20.0, -5.0, 0.0, 0.0);
      glColor3f(1.0, 1.0, 0.0);
      glScalef(0.007, 0.007, 0.007);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, a[4]);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, a[5]);
      glPopMatrix();
    }
    if(que == 2 && !winCond){
      glPushMatrix();
      glTranslatef(-9.1 + movex, 7.3, 10.5);
      glRotatef(12.0, -5.0, 0.0, 0.0);
      glColor3f(1.0, 0.0, 1.0);
      glScalef(0.006, 0.006, 0.006);
      for(omg=0;omg<26;omg++)
	glutStrokeCharacter(GLUT_STROKE_ROMAN, bq[omg]);
      glPopMatrix();

      glPushMatrix();
      glTranslatef(-4.1 + movex, 0.0, 13.8);
      glRotatef(20.0, -5.0, 0.0, 0.0);
      glColor3f(1.0, 1.0, 0.0);
      glScalef(0.007, 0.007, 0.007);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, b[0]);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, b[1]);
      glPopMatrix();

      glPushMatrix();
      glTranslatef(-0.5 + movex, 0.0, 13.8);
      glRotatef(20.0, -5.0, 0.0, 0.0);
      glColor3f(1.0, 1.0, 0.0);
      glScalef(0.007, 0.007, 0.007);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, b[2]);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, b[3]);
      glPopMatrix();

      glPushMatrix();
      glTranslatef(3.0 + movex, 0.0, 13.8);
      glRotatef(20.0, -5.0, 0.0, 0.0);
      glColor3f(1.0, 1.0, 0.0);
      glScalef(0.007, 0.007, 0.007);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, b[4]);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, b[5]);
      glPopMatrix();
    }
    if(que == 3 && !winCond){
      glPushMatrix();
      glTranslatef(-9.1 + movex, 7.3, 10.5);
      glRotatef(12.0, -5.0, 0.0, 0.0);
      glColor3f(1.0, 0.0, 1.0);
      glScalef(0.006, 0.006, 0.006);
      for(omg=0;omg<25;omg++)
	glutStrokeCharacter(GLUT_STROKE_ROMAN, cq[omg]);
      glPopMatrix();

      glPushMatrix();
      glTranslatef(-4.1 + movex, 0.0, 13.8);
      glRotatef(20.0, -5.0, 0.0, 0.0);
      glColor3f(1.0, 1.0, 0.0);
      glScalef(0.007, 0.007, 0.007);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, c[0]);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, c[1]);
      glPopMatrix();

      glPushMatrix();
      glTranslatef(-0.5 + movex, 0.0, 13.8);
      glRotatef(20.0, -5.0, 0.0, 0.0);
      glColor3f(1.0, 1.0, 0.0);
      glScalef(0.007, 0.007, 0.007);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, c[2]);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, c[3]);
      glPopMatrix();

      glPushMatrix();
      glTranslatef(3.0 + movex, 0.0, 13.8);
      glRotatef(20.0, -5.0, 0.0, 0.0);
      glColor3f(1.0, 1.0, 0.0);
      glScalef(0.007, 0.007, 0.007);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, c[4]);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, c[5]);
      glPopMatrix();
    }
  }
}

// 勝ったときに表示するメッセージ
void Congratulation()
{
  GLfloat material_specular[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
  GLfloat material_diffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
  GLfloat material_ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

  glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
  glMaterialf(GL_FRONT, GL_SHININESS, 128.0);

  glPushMatrix();
  glTranslatef(0.0 + movex, 3.0, 10.0);
  glRotatef(15.0, -5.0, 0.0, 0.0);
  glScalef(17.0, 8.0, 0.1);
  glutSolidCube(1.0);
  glPopMatrix();

  char a[]="~CONGRATULATION~";
  glPushMatrix();
  glTranslatef(-6.5 + movex, 2.3, 11.0);
  glRotatef(20.0, -5.0, 0.0, 0.0);
  glColor3f(1.0, 0.0, 1.0);
  glScalef(0.01, 0.01, 0.01);
  int i;
  for(i=0;i<16;i++)
    glutStrokeCharacter(GLUT_STROKE_ROMAN, a[i]);
  glPopMatrix();

  char d[]="HUH!!?";
  glPushMatrix();
  glTranslatef(-2.7 + movex, 4.0, 11.0);
  glRotatef(20.0, -5.0, 0.0, 0.0);
  glColor3f(1.0, 0.0, 1.0);
  glScalef(0.015, 0.015, 0.015);
  int j;
  for(j=0;j<6;j++)
    glutStrokeCharacter(GLUT_STROKE_ROMAN, d[j]);
  glPopMatrix();

  char b[]="!WOW!";
  glPushMatrix();
  glTranslatef(-4.5 + movex, 0.0, 11.0);
  glRotatef(20.0, -5.0, 0.0, 0.0);
  glColor3f(1.0, 0.0, 1.0);
  glScalef(0.01, 0.01, 0.01);
  int k;
  for(k=0;k<5;k++)
    glutStrokeCharacter(GLUT_STROKE_ROMAN, b[k]);
  glPopMatrix();

  char c[]="?WOW?";
  glPushMatrix();
  glTranslatef(1.0 + movex, 0.0, 11.0);
  glRotatef(20.0, -5.0, 0.0, 0.0);
  glColor3f(1.0, 0.0, 1.0);
  glScalef(0.01, 0.01, 0.01);
  for(k=0;k<5;k++)
    glutStrokeCharacter(GLUT_STROKE_ROMAN, c[k]);
  glPopMatrix();
}

// 負けたときに表示するメッセージ
void GameOver()
{
  GLfloat material_specular[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
  GLfloat material_diffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
  GLfloat material_ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

  glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
  glMaterialf(GL_FRONT, GL_SHININESS, 128.0);

  glPushMatrix();
  glTranslatef(0.0 + movex, 3.0, 10.0);
  glRotatef(15.0, -5.0, 0.0, 0.0);
  glScalef(17.0, 8.0, 0.1);
  glutSolidCube(1.0);
  glPopMatrix();

  char a[]="GAME";
  glPushMatrix();
  glTranslatef(-6.0 + movex, 3.0, 11.0);
  glRotatef(15.0, -5.0, 0.0, 0.0);
  glColor3f(1.0, 0.0, 1.0);
  glScalef(0.023, 0.023, 0.023);
  int i;
  for(i=0;i<4;i++)
    glutStrokeCharacter(GLUT_STROKE_ROMAN, a[i]);
  glPopMatrix();

  char b[]="OVER";
  glPushMatrix();
  glTranslatef(-1.0 + movex, 0.0, 11.0);
  glRotatef(15.0, -5.0, 0.0, 0.0);
  glColor3f(1.0, 0.0, 1.0);
  glScalef(0.023, 0.023, 0.023);
  int j;
  for(j=0;j<4;j++)
    glutStrokeCharacter(GLUT_STROKE_ROMAN, b[j]);
  glPopMatrix();
}

// ボーナス画面に表示する図（テクスチャ）
void finalScene(){

  if(miss == 0 || miss == 2){
    glPushMatrix();
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_TEXTURE_2D );
    glEnable( GL_LIGHTING );
    if(miss == 0)
      glBindTexture(GL_TEXTURE_2D, texID[3]);
    if(miss == 2)
      glBindTexture(GL_TEXTURE_2D, texID[9]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0,0.0); glVertex3f(-16.0, -12, 10.0);
    glTexCoord2f(0.0,1.0); glVertex3f(-16.0, -5, 10.0);
    glTexCoord2f(1.0,1.0); glVertex3f(-9.0, -5, 10.0);
    glTexCoord2f(1.0,0.0); glVertex3f(-9.0, -12, 10.0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
  }

  glPushMatrix();
  glEnable( GL_DEPTH_TEST );
  glEnable( GL_TEXTURE_2D );
  glEnable( GL_LIGHTING );
  if(miss == 0)
    glBindTexture(GL_TEXTURE_2D, texID[4]);
  if(miss == 1)
    glBindTexture(GL_TEXTURE_2D, texID[6]);
  if(miss == 2)
    glBindTexture(GL_TEXTURE_2D, texID[8]);
  glBegin(GL_QUADS);
  glTexCoord2f(0.0,0.0); glVertex3f(6.0, -10, 10.0);
  glTexCoord2f(0.0,1.0); glVertex3f(6.0, -1, 10.0);
  glTexCoord2f(1.0,1.0); glVertex3f(15.0, -1, 10.0);
  glTexCoord2f(1.0,0.0); glVertex3f(15.0, -10, 10.0);
  glEnd();
  glDisable(GL_TEXTURE_2D);
  glPopMatrix();

  glPushMatrix();
  glEnable( GL_DEPTH_TEST );
  glEnable( GL_TEXTURE_2D );
  glEnable( GL_LIGHTING );
  if(miss == 0)
    glBindTexture(GL_TEXTURE_2D, texID[2]);
  if(miss == 1)
    glBindTexture(GL_TEXTURE_2D, texID[5]);
  if(miss == 2)
    glBindTexture(GL_TEXTURE_2D, texID[7]);
  glBegin(GL_QUADS);
  glTexCoord2f(0.0,0.0); glVertex3f(-14.0, -3, 10.0);
  glTexCoord2f(0.0,1.0); glVertex3f(-14.0, 12, 10.0);
  glTexCoord2f(1.0,1.0); glVertex3f(1.0, 12, 10.0);
  glTexCoord2f(1.0,0.0); glVertex3f(1.0, -3, 10.0);
  glEnd();
  glDisable(GL_TEXTURE_2D);
  glPopMatrix();
}

// Retry オプションの表示
void Retry(){
  GLfloat material_specular[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
  GLfloat material_diffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
  GLfloat material_ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

  glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
  glMaterialf(GL_FRONT, GL_SHININESS, 128.0);

  glPushMatrix();
  glTranslatef(0.0 + movex, 5.0, 6.0);
  glRotatef(10.0, -5.0, 0.0, 0.0);
  glScalef(20.0, 8.0, 0.1);
  glutSolidCube(1.0);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(-10.0 + movex, -5.0, 6.0);
  glRotatef(15.0, -5.0, 0.0, 0.0);
  glScalef(15.0, 4.0, 0.1);
  glutSolidCube(1.0);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(10.0 + movex, -5.0, 6.0);
  glRotatef(15.0, -5.0, 0.0, 0.0);
  glScalef(15.0, 4.0, 0.1);
  glutSolidCube(1.0);
  glPopMatrix();

  char a[]="RETRY?";
  glPushMatrix();
  glTranslatef(-6.8 + movex, 3.1, 8.0);
  glRotatef(20.0, -5.0, 0.0, 0.0);
  glColor3f(1.0, 0.0, 1.0);
  glScalef(0.03, 0.03, 0.03);
  int i;
  for(i=0;i<6;i++)
    glutStrokeCharacter(GLUT_STROKE_ROMAN, a[i]);
  glPopMatrix();

  char b[]="YES, GOGO!";
  glPushMatrix();
  glTranslatef(-12.8 + movex, -4.0, 8.0);
  glRotatef(20.0, -5.0, 0.0, 0.0);
  glColor3f(1.0, 0.0, 1.0);
  glScalef(0.012, 0.012, 0.012);
  int j;
  for(j=0;j<10;j++)
    glutStrokeCharacter(GLUT_STROKE_ROMAN, b[j]);
  glPopMatrix();

  char c[]="NO, BUBU~";
  glPushMatrix();
  glTranslatef(3.5 + movex, -4.0, 8.0);
  glRotatef(20.0, -5.0, 0.0, 0.0);
  glColor3f(1.0, 0.0, 1.0);
  glScalef(0.012, 0.012, 0.012);
  int k;
  for(k=0;k<9;k++)
    glutStrokeCharacter(GLUT_STROKE_ROMAN, c[k]);
  glPopMatrix();

}

// Easy の選択の表示
void levelChoose1(){
  glPushMatrix();
  glTranslatef(-10.0, 0.0, 6.0);
  glRotatef(15.0, -5.0, 0.0, 0.0);
  glColor3f(0.0, 1.0, 1.0);
  glScalef(15.0, 4.0, 0.1);
  glutSolidCube(1.0);
  glPopMatrix();

  char b[]="Easy";
  glPushMatrix();
  glTranslatef(-12.8, 0.0, 8.0);
  glRotatef(20.0, -5.0, 0.0, 0.0);
  glColor3f(1.0, 0.0, 1.0);
  glScalef(0.012, 0.012, 0.012);
  int j;
  for(j=0;j<4;j++)
    glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, b[j]);
  glPopMatrix();
}

// Hard の選択の表示
void levelChoose2(){
  glPushMatrix();
  glTranslatef(10.0, 0.0, 6.0);
  glRotatef(15.0, -5.0, 0.0, 0.0);
  glColor3f(1.0, 0.0, 1.0);
  glScalef(15.0, 4.0, 0.1);
  glutSolidCube(1.0);
  glPopMatrix();

  char c[]="Hard";
  glPushMatrix();
  glTranslatef(3.5, 0.0, 8.0);
  glRotatef(20.0, -5.0, 0.0, 0.0);
  glColor3f(0.0, 1.0, 1.0);
  glScalef(0.012, 0.012, 0.012);
  int k;
  for(k=0;k<4;k++)
    glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, c[k]);
  glPopMatrix();
}

/* タイマー有無の選択 */
void timer_Choose(){
  glPushMatrix();
  glTranslatef(0.0, -9.5, 6.0);
  glRotatef(15.0, -5.0, 0.0, 0.0);
  if(timerFlag)
    glColor3f(1.0, 0.5, 0.0);
  else
    glColor3f(0.3, 0.3, 0.5);
  glScalef(20.0, 4.0, 0.1);
  glutSolidCube(1.0);
  glPopMatrix();

  int bae = 0;
  glColor3f(0.5, 1.0, 0.5);
  if(timerFlag){
    char z[]="TIMER : ON";
    glPushMatrix();
    glTranslatef(-6.5, -7.5, 8.0);
    glRotatef(20.0, -5.0, 0.0, 0.0);
    glScalef(0.012, 0.012, 0.012);
    for(bae=0;bae<10;bae++){
      if(bae < 8)
	glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, z[bae]);
      else
	glutStrokeCharacter(GLUT_STROKE_ROMAN, z[bae]);
    }
    glPopMatrix();
  }
  else{
    char z[]="TIMER : OFF";
    glPushMatrix();
    glTranslatef(-6.5, -7.5, 8.0);
    glRotatef(20.0, -5.0, 0.0, 0.0);
    glScalef(0.012, 0.012, 0.012);
    for(bae=0;bae<11;bae++){
      if(bae < 8)
	glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, z[bae]);
      else
	glutStrokeCharacter(GLUT_STROKE_ROMAN, z[bae]);
    }
    glPopMatrix();
  }
}

/* タイマーをゲームの中に表示 */
void show_Timer(){
  if(timerFlag){
    GLfloat diffuse_r = 0.0f;
    GLfloat diffuse_g = 1.0f;

    if(timer >= 40 && timer < 75){
      diffuse_r = 1.0f;
      diffuse_g = 0.5f;
    }
    if(timer >= 75){
      diffuse_r = 1.0f;
      diffuse_g = 0.0f;
    }

    GLfloat material_specular[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat material_diffuse[4] = { diffuse_r, diffuse_g, 0.0f, 1.0f };
    GLfloat material_ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
    glMaterialf(GL_FRONT, GL_SHININESS, 128.0);

    glPushMatrix();
    glTranslatef(7.0 + timerMovex + movex, 8.0, 10);
    glScalef(6.4 * progress, 0.3, 0.1);
    glutSolidCube(1.0);
    glPopMatrix();
  }
}

// ゴールモデルを描く
void show_Goal(){
  GLfloat material_specular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
  GLfloat material_diffuse[4] = { 0.0f, 0.8f, 1.0f, 1.0f };
  GLfloat material_ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

  glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
  glMaterialf(GL_FRONT, GL_SHININESS, 128.0);

  glPushMatrix();
  glTranslatef(-15+170, 2.8, 1.5);
  mySolidCylinder(0.3, 12.0, 20);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(-4+170, 2.8, 8.0);
  mySolidCylinder(0.3, 12.0, 20);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(-9.5+170, 8.0, 5.0);
  glRotatef(30.0, 0.0, -1.0, 0.0);
  glScalef(16.0, 2.5, 0.5);
  glutSolidCube(1.0);
  glPopMatrix();

  char goal[]="!!!~GOAL~!!!";
  glPushMatrix();
  glTranslatef(-16+170.0, 7.4, 1.8);
  glRotatef(30.0, 0.0, -1.0, 0.0);
  glScalef(0.01, 0.01, 0.01);
  int g;
  for(g=0;g<12;g++){
    glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, goal[g]);
  }
  glPopMatrix();
}

// 勝利負け条件
void DecideD(){
  if(miss == 3)
    loseFlag = GL_TRUE;
  if(clear == 3)
    clearStage = GL_TRUE;
}

// ゲーム終わった時に Score の表示
void loadScore(){
  glDisable(GL_LIGHTING);

  int i;

  glPushMatrix();
  glTranslatef(0.0, 0.0, 0.0);
  glRotatef(5.0, -1.0, 0.0, 0.0);
  glColor3f(0.5, 0.5, 0.5);
  glScalef(31.0, 30.0, 0.1);
  glutSolidCube(3.0);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(28.0, -18.0, 5.0);
  glColor3f(1.0, 1.0, 1.0);
  glScalef(10.0, 5.0, 0.1);
  glutSolidCube(1.0);
  glPopMatrix();

  char tap[]="Tap...";
  char finish[]="End";
  glPushMatrix();
  glTranslatef(22.0, -18.0, 6.0);
  glColor3f(0.0, 0.0, 0.0);
  glScalef(0.02, 0.02, 0.02);
  if(show_Score >= 5){
    for(i=0;i<3;i++)
      glutStrokeCharacter(GLUT_STROKE_ROMAN, finish[i]);
  }
  else{
    for(i=0;i<6;i++)
      glutStrokeCharacter(GLUT_STROKE_ROMAN, tap[i]);
  }
  glPopMatrix();

  if(show_Score >= 1){
    glPushMatrix();
    glTranslatef(18.0, -18.0, 5.0);
    glColor3f(1.0, 1.0, 1.0);
    glScalef(8.0, 5.0, 0.1);
    glutSolidCube(1.0);
    glPopMatrix();

    char skip[]="Skip";
    glPushMatrix();
    glTranslatef(14.0, -18.0, 6.0);
    glColor3f(0.0, 0.0, 0.0);
    glScalef(0.02, 0.02, 0.02);
    for(i=0;i<4;i++)
      glutStrokeCharacter(GLUT_STROKE_ROMAN, skip[i]);
    glPopMatrix();
  }

  if(show_Score >= 5 && clearStage){
    glPushMatrix();
    glTranslatef(8.0, -18.0, 5.0);
    glColor3f(1.0, 1.0, 1.0);
    glScalef(10.0, 5.0, 0.1);
    glutSolidCube(1.0);
    glPopMatrix();

    char bonus[]="Bonus";
    glPushMatrix();
    glTranslatef(3.5, -18.0, 6.0);
    glColor3f(0.0, 0.0, 0.0);
    glScalef(0.02, 0.02, 0.02);
    for(i=0;i<5;i++)
      glutStrokeCharacter(GLUT_STROKE_ROMAN, bonus[i]);
    glPopMatrix();
  }

  int score_count = 0;
  char end_miss[1], end_clear[1];
  char end_score[3] = "0";

  /*
  if(miss == 0)
    end_miss = '0';
  if(miss == 1)
    end_miss = '1';
  if(miss == 2)
    end_miss = '2';
  if(miss == 3)
    end_miss = '3';

  if(clear == 0)
    end_clear = '0';
  if(clear == 1)
    end_clear = '1';
  if(clear == 2)
    end_clear = '2';
  if(clear == 3)
    end_clear = '3';
  */

  // (sprintf) int型 を char*型に変換
  sprintf(end_miss, "%d", miss);
  sprintf(end_clear, "%d", clear);

  score_count = (((double)clear / 3) * 100) - (((double)miss / 3) * 35);
  if(score_count <= 0)
    score_count = 0;

  sprintf(end_score, "%d", score_count);

  if(show_Score >= 1){
    char a[]="Miss";
    glPushMatrix();
    glTranslatef(-22.0, 12.0, 8.0);
    glColor3f(0.5, 0.8, 0.0);
    glScalef(0.025, 0.025, 0.025);
    for(i=0;i<4;i++)
      glutStrokeCharacter(GLUT_STROKE_ROMAN, a[i]);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(4.0, 12.0, 8.0);
    glScalef(0.025, 0.025, 0.025);
    glutStrokeCharacter(GLUT_STROKE_ROMAN, end_miss[0]);
    glPopMatrix();
  }

  if(show_Score >= 2){
    char b[]="Clear";
    glPushMatrix();
    glTranslatef(-22.0, 9.0, 8.0);
    glColor3f(0.5, 0.8, 0.0);
    glScalef(0.025, 0.025, 0.025);
    for(i=0;i<5;i++)
      glutStrokeCharacter(GLUT_STROKE_ROMAN, b[i]);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(4.0, 9.0, 8.0);
    glScalef(0.025, 0.025, 0.025);
    glutStrokeCharacter(GLUT_STROKE_ROMAN, end_clear[0]);
    glPopMatrix();
  }

  if(show_Score >= 3){
    char c[]="Score";
    glPushMatrix();
    glTranslatef(-22.0, 6.0, 8.0);
    glColor3f(0.5, 0.8, 0.0);
    glScalef(0.025, 0.025, 0.025);
    for(i=0;i<5;i++)
      glutStrokeCharacter(GLUT_STROKE_ROMAN, c[i]);
    glPopMatrix();
    glPushMatrix();
    if(score_count == 100)
      glTranslatef(0.5, 6.0, 8.0);
    else if(score_count < 31)
      glTranslatef(4.0, 6.0, 8.0);
    else
      glTranslatef(2.1, 6.0, 8.0);
    glScalef(0.025, 0.025, 0.025);
    for(i=0;i<3;i++)
      glutStrokeCharacter(GLUT_STROKE_ROMAN, end_score[i]);
    glPopMatrix();
  }

  if(show_Score >= 4){
    char win1[]="   Perfect!!!      ";
    char win2[]="   Execellent!     ";
    char win3[]="   Lucky? lol      ";
    char lose[]="Try Again~ NOOB =.=";

    if(clearStage){
      glPushMatrix();
      glTranslatef(-44, -7.5, 8.0);
      glColor3f(1.0, 0.8, 1.0);
      glScalef(0.07, 0.07, 0.07);
      if(miss == 0){
	for(i=0;i<19;i++)
	  glutStrokeCharacter(GLUT_STROKE_ROMAN, win1[i]);
      }
      if(miss == 1){
	for(i=0;i<19;i++)
	  glutStrokeCharacter(GLUT_STROKE_ROMAN, win2[i]);
      }
      if(miss == 2){
	for(i=0;i<19;i++)
	  glutStrokeCharacter(GLUT_STROKE_ROMAN, win3[i]);
      }
    }
    if(loseFlag){
      glPushMatrix();
      glTranslatef(-23, -5.5, 8.0);
      //glColor3f(1.0, 0.8, 1.0);
      glColor3f(0.0, 0.1, 1.0);
      glScalef(0.03, 0.03, 0.03);
      for(i=0;i<19;i++)
	glutStrokeCharacter(GLUT_STROKE_ROMAN, lose[i]);
    }
    glPopMatrix();
  }
}

//  描画のたびに呼び出される関数 
void OurDisplay(void)
{
  //  スムーズシェイディングを指定
  glShadeModel(GL_SMOOTH);

  //  画面とデプスバッファを消去する
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //  ビューポート変換の設定 : 0 
  glMatrixMode(GL_VIEWPORT);
  glLoadIdentity();
  glViewport(0.0f, 0.0f, ourWidth, ourHeight);

  //  投影変換の設定 : 0 
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(125.0, (double)ourWidth / (double)ourHeight, 1.0f, 300.0f);

  CamX = movex;

  if(seq == 3 && clearStage == GL_TRUE){
    gluLookAt(-1.5, -1.0, 0.0, -0.5, -1.0, 0.0, 0.0, 1.0, 0.0);
  }
  else if(seq == 4 || seq == 5)
    gluLookAt(0.0, 0.0, 18.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
  else
    gluLookAt(CamX, 3, 15.0, movex, 0.0, 0.0, 0.0, 1.0, 0.0);
  //gluLookAt(-9.0, ourEyePoint.y, 15.0, -8.0, 0.0, 0.0, 0.0, 1.0, 0.0);

  //  モデル変換の設定
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glRotatef(0.0, 0.0, 1.0, 0.0);
  //glRotatef(ourEyePoint.roty, 0.0, 1.0, 0.0);

  //  光源の設定
  InitLight();
  glLineWidth(1.5);
  DecideD();

  if(seq == 0){
    startBox();
    quitBox();
  }
  if(seq == 1){
    glDisable(GL_LIGHTING);

    Reset();
    TimerReset();
    levelChoose1();
    levelChoose2();
    timer_Choose();
  }
  if(seq == 2){

    // --- humanを描画する --- //
    DrawHuman();
    mistakeBox1();
    mistakeBox2();
    show_Timer();
    show_Goal();

    if(loseFlag){
      GameOver();
      stopFlag = GL_TRUE;
    }
    else if(goalFlag){
      Congratulation();
    }
    else{
      questionBox();
      selectionAns();
      if(clearStage)
	stopFlag = GL_TRUE;
    }

    glPushMatrix();
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_TEXTURE_2D );
    glEnable( GL_LIGHTING );
    glBindTexture(GL_TEXTURE_2D, texID[1]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0,0.0); glVertex3f(-200.0, -3.25, 50.0);
    glTexCoord2f(0.0,2.5); glVertex3f(300.0, -3.25, 50.0);
    glTexCoord2f(2.5,2.5); glVertex3f(300.0, -3.25, -50.0);
    glTexCoord2f(2.5,0.0); glVertex3f(-200.0, -3.25, -50.0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
  }
  if(seq == 3){
    if(loseFlag){
      Retry();
    }
    else
      seq++;
  }
  if(seq == 4){
    loadScore();
  }
  if(seq == 5){
    if(clearStage)
      finalScene();
  }

  glFlush();

  //  ダブルバッファのバッファを入れ替える
  glutSwapBuffers();
}

// テクスチャを実装する用
void initTexturePPM(void)
{
  glGenTextures(NumOfTextureFiles, (unsigned int *)texID);

  //  テクスチャデータを読み込む関数を実行する
  int i; 
  for(i=0; i<NumOfTextureFiles; i++){
    LoadASCIIPPMImage(images[i]);

    glBindTexture(GL_TEXTURE_2D, texID[i]); 
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, imageWidth, imageHeight, 0,
		 GL_RGB, GL_UNSIGNED_BYTE, &image);
  }

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

}

//  ウインドウサイズ更新時の処理
void OurReshape(int w, int h)
{
  ourWidth = w;
  ourHeight = h;

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}

//  何もイベントがないときに呼び出される処理 
void ourIdle(void)
{
  //  アニメーションの設定
  angle = angle + 0.1;

  if(winCond){
    movex = movex + 0.5;
    if(walkTimer == 20){
      winCond = GL_FALSE;
    }
    if(movex >= 175)
      goalFlag = GL_TRUE;
  }

  //  強制的に再描画する 
  glutPostRedisplay();
}

//  glutMotionFunc()によって登録されたコールバック関数
void ourMotion(int x, int y)
{
  //  視点の設定 : 0 
  ourEyePoint.roty = -180 * ((2 * x - (double)ourWidth) / (double)ourWidth);
  ourEyePoint.y = -10 * (2 * y - (double)ourHeight) / ((double)ourHeight);

  //  強制的に再描画する 
  glutPostRedisplay();
}

//  何かキーが押されたときに実行される処理 
void myKbd(unsigned char key, int x, int y)
{
  switch(key){
    //  Esc が押されたら終了する 
  case KEY_ESC:
    exit(0);
    break;
  }
}

// マウスの入力イベント処理
void ourMouse(int button, int state, int x, int y)
{
  GLdouble model[16], proj[16];
  GLint view[4];
  GLfloat z;
  GLdouble ox, oy, oz;

  if((button==GLUT_LEFT_BUTTON) && (state==GLUT_DOWN))
    {
      glGetDoublev(GL_MODELVIEW_MATRIX, model);
      glGetDoublev(GL_PROJECTION_MATRIX, proj);
      glGetIntegerv(GL_VIEWPORT, view);

      glReadPixels(x, ourHeight-y, 1,1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);
      gluUnProject(x, ourHeight-y, z, model, proj, view, &ox, &oy, &oz);

      printf("  (%7.3f, %7.3f, %7.3f)\n", ox, oy, oz);

      /* main画面 */
      if(seq == 0){
	/* Start */
	if((ox > -37.87) && (ox < 37.87) && (oy < 19.91) && (oy > -14.02) && (oz < 0))
	  seq++;

	/* Quit */
	if((ox > -9.58) && (ox < 9.53) && (oy < -11.97) && (oy > -18.23))
	  exit(0);
      }

      /* 難易度選択画面 */
      if(seq == 1){
	/* 難易度：1 */
	if((ox > -17.31) && (ox < -2.61) && (oy < 1.69) && (oy > -1.80) && (oz > 0)){
	  level = 1;
	  seq++;
	}
	/* 難易度：2 */
	if((ox > 2.66) && (ox < 17.3) && (oy < 1.75) && (oy > -1.75) && (oz > 0)){
	  level = 2;
	  seq++;
	}
	/* タイマーの選択 */
	/* Timer : ON / OFF */
	if((ox > -9.73) && (ox < 1.63) && (oy < -6.3) && (oy > -11.27)){
	  timerFlag = !timerFlag;
	}
      }

      /* 勝った、負けた時の画面 */
      if(seq == 2 && (loseFlag || goalFlag)){
	if((ox > -8.45+movex) && (ox < 8.42+movex) && (oy < 6.77) && (oy > -0.79)){
	  //TimerReset();
	  seq++;
	}
      }

      /* (Retry / finalScene) の画面の処理 */
      if(seq == 3){
	/* Retry の処理 */
	if(loseFlag){
	  /* Yes */
	  if((ox > -17.32+movex) && (ox < -2.76+movex) && (oy < -3.35) && (oy > -6.71)){
	    seq--;
	    Reset();
	    TimerReset();
	  }
	  /* No */
	  if((ox > 2.64+movex) && (ox < 17.39+movex) && (oy < -3.29) && (oy > -6.65)){
	    seq++;
	  }
	}
      }

      /* Score を表示する画面 */
      if(seq == 4){
	/* Tap / End */
	if((ox > 23.16) && (ox < 32.88) && (oy < -15.78) && (oy > -20.29)){
	  show_Score++;
	  if(show_Score == 6){
	    seq = 0;
	    Reset();
	    TimerReset();
	  }
	}
	/* End */
	if(show_Score >= 1){
	  if((ox > 14.14) && (ox < 21.84) && (oy < -15.62) && (oy > -20.29))
	    show_Score = 5;
	}
	/* Bonus */
	if(show_Score == 5 && clearStage){
	  if((ox > 3.18) && (ox < 12.9) && (oy < -15.62) && (oy > -20.29))
	    seq++;
	}
      }

      /* finalScene の処理 */
      if(seq == 5){
	if((ox > 6.1) && (ox < 14.87) && (oy < -1.15) && (oy > -9.85)){
	  seq = 0;
	  Reset();
	  TimerReset();
	}
      }

      /* 質問の回答の選択の処理 */
      if(seq == 2 && !winCond){
	/* 選択A */
	if((ox > -4.9+movex) && (ox < -2.99+movex) && (oy < 0.38) && (oy > -0.4) && (oz > 5)){
	  if((que == 1 && level == 1) || (que == 2 && level == 2)){
	    winCond = GL_TRUE;
	    walkTimer = 0;
	    clear++;
	    que++;
	    if(que < 4)
	      TimerReset();
	  }
	  else{
	    miss++;
	  }
	}
	/* 選択B */
	if((ox > -1.0+movex) && (ox < 1.02+movex) && (oy < 0.36) && (oy > -0.4) && (oz > 5)){
	  if((que == 3 && level == 1) || (que == 1 && level == 2)){
	    winCond = GL_TRUE;
	    walkTimer = 0;
	    clear++;
	    que++;
	    if(que < 4)
	      TimerReset();
	  }
	  else{
	    miss++;
	  }
	}
	/* 選択C */
	if((ox > 2.97+movex) && (ox < 5.01+movex) && (oy < 0.38) && (oy > -0.4) && (oz > 5)){
	  if((que == 2 && level == 1) || (que == 3 && level == 2)){
	    winCond = GL_TRUE;
	    walkTimer = 0;
	    clear++;
	    que++;
	    if(que < 4)
	      TimerReset();
	  }
	  else{
	    miss++;
	  }
	}
      }
    }

  glutPostRedisplay();
}

// タイマー処理用
void displayTimer(int value)
{
  glutPostRedisplay();

  walkTimer++;

  if(!winCond && !stopFlag && timerFlag){
    timer++;

    if(timer == 0)
      progress = 1.0;
    if(timer > 0 && timer <= 100){
      progress = (100.0 - timer) / 100.0;
      timerMovex += 0.032;
    }
    if(timer > 100){
      progress = 0.0;
      loseFlag = GL_TRUE;
    }
  }

  glutTimerFunc(100,displayTimer,0);
}

//  main関数
int main(int argc, char *argv[])
{
  glutInit(&argc, argv);

  //  ダブルバッファを用意
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

  glutInitWindowSize(ourWidth, ourHeight);
  glutCreateWindow("human");

  glutKeyboardFunc(myKbd);	//  キーボードイベント
  glutDisplayFunc(OurDisplay);	//  描画 
  glutReshapeFunc(OurReshape);	//  ウィンドウサイズ変更 
  glutIdleFunc(ourIdle);        //  何もないときに実行される

  //  マウスドラックイベント処理のためのコールバックを登録	
  glutMotionFunc(ourMotion);

  //マウスボタンイベント処理のためのコールバック関数を定義
  glutMouseFunc(ourMouse);

  initTexturePPM();

  glutTimerFunc(1000,displayTimer,0);

  //  デプスバッファを使うように設定
  glEnable(GL_DEPTH_TEST);

  glutMainLoop();

  return 0;
}

/*  --- 以降，テクスチャファイルを読み込むために必要な関数 ---  */
char* get_word(void)
{
  while(1)
    {
      if(*ptr == '\0')
	{
	  fgets(readbuf, 256, texturefile);
	  while(*readbuf == '#')
	    {
	      fgets(readbuf, 256, texturefile);
	    }
	  ptr = readbuf;
	  continue;
	}
      else if(isspace(*ptr))
	{
	  ptr++;
	  continue;
	}
      else
	{
	  break;
	}
    }

  ptr2 = word;
  while(!isspace(*ptr))
    {
      *ptr2++ = *ptr++;
    }
  *ptr2 = '\0';
  return word;
}


void LoadASCIIPPMImage(const char* filename)
{
  int i, j;

  texturefile = fopen(filename, "r");

  get_word();

  for (i = imageHeight-1; i >=0 ; i--) 
    {
      for (j = 0; j < imageWidth; j++) 
	{
	  image[i][j][0] = (GLubyte) atoi(get_word());
	  image[i][j][1] = (GLubyte) atoi(get_word());
	  image[i][j][2] = (GLubyte) atoi(get_word());
	}
    }
  fclose(texturefile);
}
