/* 3D viewer per Tuki3
   Copyright (C) 2018-2020  Valentina Sisini
   francescomichelesisini@gmail.com
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */
#include "tuki3.h"
#include "Utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

//gcc -o c4 chapter.4.1.c Utils.c -lglut -lGL -lGLU -lGLEW -lm

#define WINDOW_TITLE_PREFIX "Tuki 3"
#define N_BUFFERS (NUMERO_OGGETTI+1+1+1)*2 // (oggetti+tuki+ground+traguardo)*2

int
  CurrentWidth = 800,
  CurrentHeight = 400,
  WindowHandle = 0;
unsigned FrameCount = 0;

GLuint
  ProjectionMatrixUniformLocation,
  ViewMatrixUniformLocation,
  ModelMatrixUniformLocation,
  VAO[N_BUFFERS/2]={ 0 },
  vertex_in_buffer[N_BUFFERS/2];
  BufferIds[N_BUFFERS] = { 0 },
  ShaderIds[3] = { 0 };

Matrix
  ProjectionMatrix,
  ViewMatrix,
  ModelMatrix;

float CubeRotation = 0;
float CubePosition = 0;
clock_t LastTime = 0;

/*** Variabili View ***/
float rot_x = -0.05;
float rot_y = 0;
float rot_z = 0;
float trasla_x = 0.0;
float trasla_y = 0.2;
float trasla_z = -0.2;

/*** Variabili del gioco ***/
extern Giocatore gctr_tuki;
extern oggetto ob[NUMERO_OGGETTI];
extern azione act;


/*** Orientazione Tuki ***/
float r_y = 0;
float r_x = 0;
float r_z = 0;
#define MAX_X PI/4.
#define MAX_Y PI/8.
#define MAX_Z PI/8.

void Initialize(int, char*[]);
void InitWindow(int, char*[]);
void ResizeFunction(int, int);
void RenderFunction(void);
void TimerFunction(int);
void IdleFunction(void);
void crea_tuki(void);
void crea_object(int i);
void crea_ground(void);
void crea_target(void);

void DestroyCube(void);
void draw_tuki(void);
void draw_object(int o_index);
void draw_ground(void);
void draw_target(void);
int main_controller();
void keyboard(unsigned char key, int x, int y);

void Initialize(int argc, char* argv[])
{
  GLenum GlewInitResult;

  InitWindow(argc, argv);

  GlewInitResult = glewInit();

  if (GLEW_OK != GlewInitResult) {
    fprintf(
      stderr,
      "ERROR: %s\n",
      glewGetErrorString(GlewInitResult)
    );
    exit(EXIT_FAILURE);
  }

  fprintf(
    stdout,
    "INFO: OpenGL Version: %s\n",
    glGetString(GL_VERSION)
  );
  glGetError();
  glClearColor(0.1f, 0.1f, 0.1f, 1.f);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
    
  //glEnable(GL_CULL_FACE);
  //glCullFace(GL_BACK);
  glFrontFace(GL_CCW);
    
  ModelMatrix = IDENTITY_MATRIX;
  ProjectionMatrix = IDENTITY_MATRIX;
  ViewMatrix = IDENTITY_MATRIX;
  TranslateMatrix(&ViewMatrix, 3, 0, -5);

  /* Vengono creati Campo, Personaggi e Oggetti */
  crea_tuki();
  crea_ground();
  crea_target();
  for(int i=0;i<NUMERO_OGGETTI;i++)
    {
      crea_object(i);
    }
}

void InitWindow(int argc, char* argv[])
{
  glutInit(&argc, argv);
  
  glutInitContextVersion(4, 0);
  glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
  glutInitContextProfile(GLUT_CORE_PROFILE);

  glutSetOption(
    GLUT_ACTION_ON_WINDOW_CLOSE,
    GLUT_ACTION_GLUTMAINLOOP_RETURNS
  );
  
  glutInitWindowSize(CurrentWidth, CurrentHeight);

  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);

  WindowHandle = glutCreateWindow(WINDOW_TITLE_PREFIX);

  if(WindowHandle < 1) {
    fprintf(
      stderr,
      "ERROR: Could not create a new rendering window.\n"
    );
    exit(EXIT_FAILURE);
  }
  glutPositionWindow(0,600);
  glutReshapeFunc(ResizeFunction);
  glutDisplayFunc(RenderFunction);
  glutIdleFunc(main_controller);
  glutTimerFunc(0, TimerFunction, 0);
  glutKeyboardFunc(keyboard);
}

void ResizeFunction(int Width, int Height)
{
  CurrentWidth = Width;
  CurrentHeight = Height;
  glViewport(0, 0, CurrentWidth, CurrentHeight);
  ProjectionMatrix =
  CreateProjectionMatrix(
    60,
    (float)CurrentWidth / CurrentHeight,
    1.0f,
    100.0f
			 ); 
  
  glUseProgram(ShaderIds[0]);
  glUniformMatrix4fv(ProjectionMatrixUniformLocation, 1, GL_FALSE, ProjectionMatrix.m);
  glUseProgram(0);
}

void RenderFunction(void)
{
++FrameCount;
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
glClearColor(.2,.2,.2,1);

/*gluLookAt(
  0,      0,      1.0,
  0,   0, 1.0,
  0.0,    0.0,    1.0);
*/
ViewMatrix = IDENTITY_MATRIX;
RotateAboutX(&ViewMatrix,rot_x);
RotateAboutY(&ViewMatrix,rot_y);
RotateAboutZ(&ViewMatrix,rot_z);
TranslateMatrix(&ViewMatrix,-(gctr_tuki.pos_x/(double)LUNGHEZZA_CAMMINO), trasla_y, trasla_z);
ScaleMatrix(&ViewMatrix,8,8,8);

//Disegna:
 draw_tuki();
 for(int i=0;i<NUMERO_OGGETTI;i++)
   {
     draw_object(i);
   }
 draw_ground();
 draw_target();
 glutSwapBuffers();
 glutPostRedisplay();
}

void IdleFunction(void)
{
  //glutPostRedisplay();
}

void TimerFunction(int Value)
{
  if (0 != Value) {
    char* TempString = (char*)
      malloc(512 + strlen(WINDOW_TITLE_PREFIX));
    char *a;
    if (act == PRENDI) a = "PRENDI";
    if (act == MANGIA) a = "MANGIA";
    if (act == SALTA) a = "SALTA";
    sprintf(
      TempString,
      "%s: Azione: %s",
      WINDOW_TITLE_PREFIX,
      a
    );

    glutSetWindowTitle(TempString);
    free(TempString);
  }
  
  FrameCount = 0;
  glutTimerFunc(250, TimerFunction, 1);
}

void crea_tuki(void)
{
  float offset = 0;//Tuki appoggia i piedi a 0
  

  const Vertex VERTICES[24] =
    {
      
      { { 16./33.+offset,19./33.+offset,0, 1 }, {0 , 0, 0, 1 } }, //A
      { { 9./33.+offset,18./33.+offset,0, 1 }, { 0, 0, 0, 1 } }, //B
      { {  20./33.+offset,18./33.+offset,0, 1 }, { 0, 0, 0, 1 } },//C
      { {  12.5/33.+offset,17./33.+offset,0, 1 }, { 0, 0, 0, 1 } },//D
      { { 18./33.+offset,17./33.+offset,0, 1 }, { 0, 102./255., 255./255, 1 } },//E
      { { 16./33.+offset,16./33.+offset,0, 1 }, { 0, 102./255., 255./255, 1 } },//F
      { {  28./33.+offset,16./33.+offset,0, 1 }, { 1, 1, 0, 1 } },//G
      { {  16.5/33.+offset,13./33.+offset,0, 1 }, { 1, 1, 0, 1 } },//H
      { {  32.9/33.+offset,13./33.+offset,0, 1 }, { 1, 1, 0, 1 } },//I
      { {  10./33.+offset,12./33.+offset,0, 1 }, { 0, 0, 0, 1 } },//J
      { {  28./33.+offset,11./33.+offset,0, 1 }, { 1, 1, 0, 1 } },//K
      { {  9./33.+offset,10./33.+offset,0, 1 }, { 0, 0, 0, 1 } },//L
      { {  17./33.+offset,10./33.+offset,0, 1 }, { 1, 1, 1, 1 } },//M
      { {  32./33.+offset,10./33.+offset,0, 1 }, { 0, 0, 1, 1 } },//N
      { {  13./33.+offset,5./33.+offset,0, 1 }, { 0, 0, 0, 1 } },//O
      { {  0./33.+offset,4./33.+offset,0, 1 }, { 0, 0, 0, 1 } },//P
      { {  10./33.+offset,4./33.+offset,0, 1 }, { 0, 0, 0, 1 } },//Q
      { {  12./33.+offset,4./33.+offset,0, 1 }, { 0, 0, 0, 1 } },//R
      { {  14./33.+offset,4./33.+offset,0, 1 }, { 0, 0, 0, 1 } },//S
      { {  13.5/33.+offset,2./33.+offset,0, 1 }, { 0, 0, 0, 1 } },//T
      { {  6./33.+offset,0./33.+offset,0, 1 }, { 0, 0, 0, 1 } },//U
      { {  10./33.+offset,0./33.+offset,0, 1 }, { 0, 0, 0, 1 } },//V
      { {  13./33.+offset,0./33.+offset,0, 1 }, { 0, 0, 0, 1 } },//W
      { {  16./33.+offset,0./33.+offset,0, 1 }, { 0, 0, 0, 1 } }//X
    };
 
  const GLuint INDICES[57] =
    {
            
      0,1,3,
      0,3,5,
      0,5,4,
      0,4,2,
      2,10,6,
      6,10,8,
      10,13,8,
      7,10,2,
      7,2,5,
      1,9,3,
      3,12,5,
      11,12,3,
      11,17,12,
      15,17,11,
      20,21,16,
      21,17,16,
      17,18,14,
      17,22,18,
      22,23,19
      
    };

  
  ShaderIds[0] = glCreateProgram();
  ExitOnGLError("ERROR: Could not create the shader program, stronzo");

  ShaderIds[1] = LoadShader("SimpleShader.fragment.glsl", GL_FRAGMENT_SHADER);
  ShaderIds[2] = LoadShader("SimpleShader.vertex.glsl", GL_VERTEX_SHADER);
  glAttachShader(ShaderIds[0], ShaderIds[1]);
  glAttachShader(ShaderIds[0], ShaderIds[2]);
  
  glLinkProgram(ShaderIds[0]);
  ExitOnGLError("ERROR: Could not link the shader program");

  ModelMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "ModelMatrix");
  ViewMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "ViewMatrix");
  ProjectionMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "ProjectionMatrix");
  ExitOnGLError("ERROR: Could not get the shader uniform locations");

  glGenBuffers(2, &BufferIds[0]);
  ExitOnGLError("ERROR: Could not generate the buffer objects");

  glGenVertexArrays(1, &VAO[0]);
  ExitOnGLError("ERROR: Could not generate the VAO");
  glBindVertexArray(VAO[0]);
  ExitOnGLError("ERROR: Could not bind the VAO");

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  ExitOnGLError("ERROR: Could not enable vertex attributes");

  glBindBuffer(GL_ARRAY_BUFFER, BufferIds[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);
  ExitOnGLError("ERROR: Could not bind the VBO to the VAO");
  
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)0);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)sizeof(VERTICES[0].Position));
  ExitOnGLError("ERROR: Could not set VAO attributes");

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferIds[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(INDICES), INDICES, GL_STATIC_DRAW);
  ExitOnGLError("ERROR: Could not bind the IBO to the VAO");
  
  glBindVertexArray(0);
}


void crea_ground(void)
{
  const Vertex VERTICES[4] =
    {
      {{-1,-0.2,-1,1},{153./255,51./255,0,1}},
      {{1,-0.2,-1,1},{153./255,51./255,0,1}},
      {{1,-0.2,1,1},{153./255,51./255,0,1}},
      {{-1,-0.2,1,1},{153./255,51./255,0,1}}
    };
  
  const GLuint INDICES[12] =
    {
      0,1,2, 2,3,0,
      0,3,2, 2,1,0
    };
  ShaderIds[0] = glCreateProgram();
  ExitOnGLError("ERROR: Could not create the shader program, cazzo");

  ShaderIds[1] = LoadShader("SimpleShader.fragment.glsl", GL_FRAGMENT_SHADER);
  ShaderIds[2] = LoadShader("SimpleShader.vertex.glsl", GL_VERTEX_SHADER);
  glAttachShader(ShaderIds[0], ShaderIds[1]);
  glAttachShader(ShaderIds[0], ShaderIds[2]);
  
  glLinkProgram(ShaderIds[0]);
  ExitOnGLError("ERROR: Could not link the shader program, minchia");

  ModelMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "ModelMatrix");
  ViewMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "ViewMatrix");
  ProjectionMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "ProjectionMatrix");
  ExitOnGLError("ERROR: Could not get the shader uniform locations");

  glGenBuffers(2, &BufferIds[2]);
  ExitOnGLError("ERROR: Could not generate the buffer objects");

  glGenVertexArrays(1, &VAO[1]);
  ExitOnGLError("ERROR: Could not generate the VAO");
  glBindVertexArray(VAO[1]);
  ExitOnGLError("ERROR: Could not bind the VAO");

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  ExitOnGLError("ERROR: Could not enable vertex attributes");

  glBindBuffer(GL_ARRAY_BUFFER, BufferIds[2]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);
  ExitOnGLError("ERROR: Could not bind the VBO to the VAO");
  
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)0);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)sizeof(VERTICES[0].Position));
  ExitOnGLError("ERROR: Could not set VAO attributes");

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferIds[3]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(INDICES), INDICES, GL_STATIC_DRAW);
  ExitOnGLError("ERROR: Could not bind the IBO to the VAO");
  
  glBindVertexArray(0);
}


void crea_target(void)
{
  double scale = 8.;
  const Vertex VERTICES[12] =
    {
      {{0,0,0,1},{1.,1.,1.,1}},//0
      {{0,0,1./scale,1}, {1.,1.,1.,1}},//1
      {{0,5./scale,1./scale,1},{1.,1.,1.,1}},//2
      {{0,5./scale,0,1},{1.,1.,1.,1}},//3
      {{0,3./scale,1./scale,1},{1.,0.,0.,1}},//4
      {{0,5./scale,4./scale,1},{1.,1.,1.,1}},//5
      {{0,3./scale,4./scale,1},{1.,1.,1.,1}},//6
      {{0,3./scale,7./scale,1},{1.,0.,0.,1}},//7
      {{0,5./scale,7./scale,1},{1.,1.,1.,1}},//8
      {{0,0./scale,7./scale,1},{1.,1.,1.,1}},//9
      {{0,0./scale,8./scale,1},{1.,1.,1.,1}},//10
      {{0,5./scale,8./scale,1},{1.,1.,1.,1}}//11
    };
  
  const GLuint INDICES[24] =
    {
      0,1,2,
      2,3,0,
      2,4,6,
      6,5,2,
      6,7,8,
      8,5,6,
      8,9,10,
      8,10,11
    };
  ShaderIds[0] = glCreateProgram();
  ExitOnGLError("ERROR: Could not create the shader program, cazzo");

  ShaderIds[1] = LoadShader("SimpleShader.fragment.glsl", GL_FRAGMENT_SHADER);
  ShaderIds[2] = LoadShader("SimpleShader.vertex.glsl", GL_VERTEX_SHADER);
  glAttachShader(ShaderIds[0], ShaderIds[1]);
  glAttachShader(ShaderIds[0], ShaderIds[2]);
  
  glLinkProgram(ShaderIds[0]);
  ExitOnGLError("ERROR: Could not link the shader program, minchia");

  ModelMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "ModelMatrix");
  ViewMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "ViewMatrix");
  ProjectionMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "ProjectionMatrix");
  ExitOnGLError("ERROR: Could not get the shader uniform locations");

  glGenBuffers(2, &BufferIds[4]);
  ExitOnGLError("ERROR: Could not generate the buffer objects");

  glGenVertexArrays(1, &VAO[2]);
  ExitOnGLError("ERROR: Could not generate the VAO");
  glBindVertexArray(VAO[2]);
  ExitOnGLError("ERROR: Could not bind the VAO");

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  ExitOnGLError("ERROR: Could not enable vertex attributes");

  glBindBuffer(GL_ARRAY_BUFFER, BufferIds[4]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);
  ExitOnGLError("ERROR: Could not bind the VBO to the VAO");
  
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)0);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)sizeof(VERTICES[0].Position));
  ExitOnGLError("ERROR: Could not set VAO attributes");

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferIds[5]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(INDICES), INDICES, GL_STATIC_DRAW);
  ExitOnGLError("ERROR: Could not bind the IBO to the VAO");
  
  glBindVertexArray(0);
}

void crea_object(int i)
{

  ShaderIds[0] = glCreateProgram();
  ExitOnGLError("ERROR: Could not create the shader program, stronzo");

  ShaderIds[1] = LoadShader("SimpleShader.fragment.glsl", GL_FRAGMENT_SHADER);
  ShaderIds[2] = LoadShader("SimpleShader.vertex.glsl", GL_VERTEX_SHADER);
  glAttachShader(ShaderIds[0], ShaderIds[1]);
  glAttachShader(ShaderIds[0], ShaderIds[2]);
  
  glLinkProgram(ShaderIds[0]);
  ExitOnGLError("ERROR: Could not link the shader program");

  ModelMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "ModelMatrix");
  ViewMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "ViewMatrix");
  ProjectionMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "ProjectionMatrix");
  ExitOnGLError("ERROR: Could not get the shader uniform locations");

  glGenBuffers(2, &BufferIds[6+i*2]);
  ExitOnGLError("ERROR: Could not generate the buffer objects");

  glGenVertexArrays(1, &VAO[3+i]);
  ExitOnGLError("ERROR: Could not generate the VAO");
  glBindVertexArray(VAO[3+i]);
  ExitOnGLError("ERROR: Could not bind the VAO");

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  ExitOnGLError("ERROR: Could not enable vertex attributes");
  
  //Determina il tipo di oggetto da creare
  if(ob[i].t == GEMMA)
    {
      
      float y1,y2,y3;
      y3 = 1;
      y2 = 0.6;
      y1 = 0;
      float norm=8.;
      
      const Vertex VERTICES[13] =
	{
	  //Piano superiore del diamante
	  { { 2./norm, y3,  3./norm, 1 }, { 0, 1, 204./255., 1 } },
	  { { 3./norm,  y3,  4./norm, 1 }, { 0, 0, 1, 1 } },
	  { { 5./norm,  y3,  4./norm, 1 }, { 0, 1, 204./255., 1 } },
	  { { 6./norm, y3,  3./norm, 1 }, { 0, 0, 1, 1 } },
	  { { 5./norm, y3, 2./norm, 1 }, { 0, 1, 204./255., 1 } },
	  { { 3./norm, y3, 2./norm, 1 }, { 0, 0, 1, 1 } },
	  //Piano di mezzo
	  { { 0./norm, y2,  3./norm, 1 }, { 0, 0, 1, 1 } },
	  { { 2./norm,  y2,  6./norm, 1 }, { 0, 0, 1, 1 } },
	  { { 6./norm,  y2,  6./norm, 1 }, { 0, 1, 204./255., 1 } },
	  { { 8./norm, y2,  3./norm, 1 }, { 0, 1, 204./255., 1 } },
	  { { 6./norm, y2, 0./norm, 1 }, { 0, 0, 1, 1 } },
	  { { 2./norm, y2, 0./norm, 1 }, { 0, 0, 1, 1 } },
	  //Vertice
	  { { 4./norm, y1, 3./norm, 1 }, { 1, 1, 1, 1 } }
	};
      
      vertex_in_buffer[i]=66;
      const GLuint INDICES[66] =
	{
	  //Faccia superiore
	  0,5,1,
	  //Facce laterali superiori
	  1,5,4,
	  1,4,2,
	  2,4,3,
	  6,5,0,
	  6,11,5,
	  5,11,10,
	  5,10,4,
	  4,10,9,
	  4,9,3,
	  3,9,2,
	  2,9,8,
	  2,8,7,
	  1,2,7,
	  7,0,1,
	  7,6,0,
	  //Facce laterali inferiori
	  6,12,11,
	  11,12,10,
	  10,12,9,
	  9,12,8,
	  8,12,7,
	  7,12,6
	};
    
  
  

      glBindBuffer(GL_ARRAY_BUFFER, BufferIds[6+i*2]);
      glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);
      ExitOnGLError("ERROR: Could not bind the VBO to the VAO");
      
      glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)0);
      glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)sizeof(VERTICES[0].Position));
      ExitOnGLError("ERROR: Could not set VAO attributes");
      
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferIds[7+i*2]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(INDICES), INDICES, GL_STATIC_DRAW);
      ExitOnGLError("ERROR: Could not bind the IBO to the VAO");
      
      glBindVertexArray(0);
    }
  
  //Pozione
   if(ob[i].t == POZIONE)
    {
      float r,g,b;
      r=205./255;
      g=205./255;
      b=255./255;
      float y1,y2,y3;
      y3 = 1;
      y2 = 0.4;
      y1 = 0;
      float norm=5.;
      
      const Vertex VERTICES[12] =
	{
	  //Base della pozione
	  { { 0./norm, y1,  0./norm, 1 }, { r, g, b, 1 } },
	  { { 5./norm,  y1,  0./norm, 1 }, { r, g, b, 1 } },
	  { { 5./norm,  y1,  5./norm, 1 }, { r, g, b, 1 } },
	  { { 0./norm, y1,  5./norm, 1 }, { r, g, b, 1 } },
	  //Collo
	  { { 3./norm, y2, 2./norm, 1 }, { r, g, b, 1 } },
	  { { 4./norm, y2, 2./norm, 1 }, { r, g, b, 1 } },
	  { { 4./norm, y2, 3./norm, 1 }, { r, g, b, 1 } },
	  { { 3./norm, y2, 3./norm, 1 }, { r, g, b, 1 } },
	  //Tappo
	  { { 3./norm, y3, 2./norm, 1 }, { r, g, b, 1 } },
	  { { 4./norm, y3, 2./norm, 1 }, { r, g, b, 1 } },
	  { { 4./norm, y3, 3./norm, 1 }, { r, g, b, 1 } },
	  { { 3./norm, y3, 3./norm, 1 }, { r, g, b, 1 } }
	};
      
      vertex_in_buffer[i]=54;
      const GLuint INDICES[54] =
	{
	  //tappo
	  8,9,10,
	  10,11,8,
	  8,4,9,
	  4,5,9,
	  5,9,10,
	  1,6,5,
	  7,10,11,
	  11,6,7,
	  4,8,11,
	  11,7,4,
	  0,5,4,
	  0,1,5,
	  5,1,6,
	  1,2,6,
	  6,2,7,
	  7,2,3,
	  7,3,4,
	  4,3,0
	};
    
  
  

      glBindBuffer(GL_ARRAY_BUFFER, BufferIds[6+i*2]);
      glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);
      ExitOnGLError("ERROR: Could not bind the VBO to the VAO");
      
      glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)0);
      glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)sizeof(VERTICES[0].Position));
      ExitOnGLError("ERROR: Could not set VAO attributes");
      
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferIds[7+i*2]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(INDICES), INDICES, GL_STATIC_DRAW);
      ExitOnGLError("ERROR: Could not bind the IBO to the VAO");
      
      glBindVertexArray(0);
    }

   //MELANZANA
   if(ob[i].t == MELANZANA)
    {
      float r,g,b;
      r = 184./255;
      g = 0./255;
      b = 230./255;
      r=0;
      g=1;
      b=0;
      float y1,y2,y3,y4;
      y4 = 1.;
      y3 = 0.8;
      y2 = 0.3;
      y1 = 0;
      float norm=4.;
      
      const Vertex VERTICES[8] =
	{
	  //Base melanzana
	  { { 2./norm, y1,  2./norm, 1 }, { r, g, b, 1 } },
	  
	  //Pancia
	  { { 2./norm, y2, 0./norm, 1 }, { r, g, b, 1 } },
	  { { 4./norm, y2, 2./norm, 1 }, { r, g, b, 1 } },
	  { { 2./norm, y2, 4./norm, 1 }, { r, g, b, 1 } },
	  { { 0./norm, y2, 2./norm, 1 }, { r, g, b, 1 } },
	  //Sommità
	  { { 2./norm, y3, 0./norm, 1 }, { r, g, b, 1 } },
	  
	  //Picciuolo
	  { { 1./norm, y4, 2./norm, 1 }, { 0, 1, 0, 1 } },
	  { { 2./norm, y4, 2./norm, 1 }, { 0, 1, 0, 1 } }
	};
      
      vertex_in_buffer[i]=27;
      const GLuint INDICES[27] =
	{
	  4,0,1,
	  1,2,3,
	  2,0,3,
	  3,0,4,
	  5,1,4,
	  5,4,3,
	  5,3,2,
	  5,2,1,
	  6,5,7
	  
	};
    
  
  

      glBindBuffer(GL_ARRAY_BUFFER, BufferIds[6+i*2]);
      glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);
      ExitOnGLError("ERROR: Could not bind the VBO to the VAO");
      
      glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)0);
      glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)sizeof(VERTICES[0].Position));
      ExitOnGLError("ERROR: Could not set VAO attributes");
      
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferIds[7+i*2]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(INDICES), INDICES, GL_STATIC_DRAW);
      ExitOnGLError("ERROR: Could not bind the IBO to the VAO");
      
      glBindVertexArray(0);
    }
}



void draw_tuki(void)
{
  float CubeAngle;
  static float inc_x = 0.1;
  static float inc_z = 0.2;
  static float inc_y = 0.1;
  /* Calcola la camminata */
  r_y+= inc_y;
  if(r_y >= MAX_Y || r_y<=-MAX_X) inc_y = -inc_y;
  r_z += inc_z;
  if(r_z >= MAX_Z || r_z<=-MAX_Z) inc_z = -inc_z;
  
  ModelMatrix = IDENTITY_MATRIX;
  //RotateAboutY(&ModelMatrix, CubeAngle);
  RotateAboutY(&ModelMatrix, r_y);
  RotateAboutZ(&ModelMatrix, r_z);
  ScaleMatrix(&ModelMatrix, 0.05,0.05, 0);
  TranslateMatrix(&ModelMatrix, gctr_tuki.pos_x/(double)LUNGHEZZA_CAMMINO, -0.19, 0);
  
  glUseProgram(ShaderIds[0]);
  ExitOnGLError("ERROR: Could not use the shader program, cazzo");
  
  glUniformMatrix4fv(ModelMatrixUniformLocation, 1, GL_FALSE, ModelMatrix.m);
  glUniformMatrix4fv(ViewMatrixUniformLocation, 1, GL_FALSE, ViewMatrix.m);
  ExitOnGLError("ERROR: Could not set the shader uniforms");

  glBindVertexArray(VAO[0]);
  ExitOnGLError("ERROR: Could not bind the VAO for drawing purposes");
 
  glDrawElements(GL_TRIANGLES,57, GL_UNSIGNED_INT, (GLvoid*)0);
  ExitOnGLError("ERROR: Could not draw the cube");
  
  glBindVertexArray(0);
  glUseProgram(0);
      
}




void draw_ground(void)
{
  float CubeAngle;

  ModelMatrix = IDENTITY_MATRIX;
  //RotateAboutZ(&ModelMatrix, PI/2.);
  //RotateAboutX(&ModelMatrix, PI/9.);
  ScaleMatrix(&ModelMatrix, 10,1, 2);
  glUseProgram(ShaderIds[0]);
  ExitOnGLError("ERROR: Could not use the shader program, cazzo");
  
  glUniformMatrix4fv(ModelMatrixUniformLocation, 1, GL_FALSE, ModelMatrix.m);
  glUniformMatrix4fv(ViewMatrixUniformLocation, 1, GL_FALSE, ViewMatrix.m);
  ExitOnGLError("ERROR: Could not set the shader uniforms");

  glBindVertexArray(VAO[1]);
  ExitOnGLError("ERROR: Could not bind the VAO for drawing purposes");
  
  glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, (GLvoid*)0);
  ExitOnGLError("ERROR: Could not draw the Ground");
  
  glBindVertexArray(1);
  glUseProgram(0);
  
    
}

void draw_target(void)
{
  float CubeAngle;

  ModelMatrix = IDENTITY_MATRIX;
  //RotateAboutZ(&ModelMatrix, PI/2.);
  //RotateAboutX(&ModelMatrix, PI/9.);
  ScaleMatrix(&ModelMatrix, 0.7,0.7, 0.7);
  TranslateMatrix(&ModelMatrix, 1,-0.17, -0.5);
  glUseProgram(ShaderIds[0]);
  ExitOnGLError("ERROR: Could not use the shader program, cazzo");
  
  glUniformMatrix4fv(ModelMatrixUniformLocation, 1, GL_FALSE, ModelMatrix.m);
  glUniformMatrix4fv(ViewMatrixUniformLocation, 1, GL_FALSE, ViewMatrix.m);
  ExitOnGLError("ERROR: Could not set the shader uniforms");

  glBindVertexArray(VAO[2]);
  ExitOnGLError("ERROR: Could not bind the VAO for drawing purposes");
  
  glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_INT, (GLvoid*)0);
  ExitOnGLError("ERROR: Could not draw the Ground");
  
  glBindVertexArray(1);
  glUseProgram(0);
  
    
}

void draw_object(int o_index)
{
  float CubeAngle;
  int i = o_index;
  if(!ob[i].exists) return;
    
  ModelMatrix = IDENTITY_MATRIX;
  //RotateAboutY(&ModelMatrix, CubeAngle);
  ScaleMatrix(&ModelMatrix, 0.02,0.02, 0.02);
  TranslateMatrix(&ModelMatrix, ob[i].pos_x/(double)LUNGHEZZA_CAMMINO,-0.195, -0.02);
  
    
  glUseProgram(ShaderIds[0]);
  ExitOnGLError("ERROR: Could not use the shader program, cazzo");
  
  glUniformMatrix4fv(ModelMatrixUniformLocation, 1, GL_FALSE, ModelMatrix.m);
  glUniformMatrix4fv(ViewMatrixUniformLocation, 1, GL_FALSE, ViewMatrix.m);
  ExitOnGLError("ERROR: Could not set the shader uniforms");

  glBindVertexArray(VAO[3+i]);
  ExitOnGLError("ERROR: Could not bind the VAO for drawing purposes");
  
  glDrawElements(GL_TRIANGLES, vertex_in_buffer[i], GL_UNSIGNED_INT, (GLvoid*)0);
  ExitOnGLError("ERROR: Could not draw the cube");
  
  glBindVertexArray(2+i);
  glUseProgram(0);
  
    
}

void keyboard(unsigned char key, int x, int y)
{

	if(key=='x')
	{
	  rot_x-=0.1;
	  
	  glutPostRedisplay();
	}
	else if(key=='y')
	{
	  rot_y+=0.1;
	  
	  glutPostRedisplay();
	}
	else if(key=='z')
	{
	  rot_z+=0.1;
	  
	  glutPostRedisplay();
	}
	
	else if(key=='i')
	  {
	    trasla_z+=0.01;
	    glutPostRedisplay();
	  }
	else if(key=='o')
	  {
	    trasla_z-=0.01;
	    glutPostRedisplay();
	  }
	else if(key=='m')
	  {
	    act = MANGIA;
	  }
	else if(key=='s')
	  {
	    act = SALTA;
	  }
	else if(key=='p')
	  {
	    act = PRENDI;
	  }
	


}
