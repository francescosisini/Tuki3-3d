#include "tuki3.h"
#include "Utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

//gcc -o c4 chapter.4.1.c Utils.c -lglut -lGL -lGLU -lGLEW -lm

#define WINDOW_TITLE_PREFIX "Chapter 1"
#define N_BUFFERS (NUMERO_OGGETTI+1+1)*2 // (oggetti+tuki+ground)*2

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
float spin_x = 0;
float spin_y = 0;
float spin_z = 0;
float translate_x = 0.0;
float translate_y = 0.0;
float translate_z = -0.5;

/*** Variabili del gioco ***/
extern Giocatore gctr_tuki;
extern oggetto ob[NUMERO_OGGETTI]; 


void Initialize(int, char*[]);
void InitWindow(int, char*[]);
void ResizeFunction(int, int);
void RenderFunction(void);
void TimerFunction(int);
void IdleFunction(void);
void create_tuki(void);
void create_object(int i);
void create_ground(void);
void DestroyCube(void);
void draw_tuki(void);
void draw_object(int o_index);
void draw_ground(void);
int main_controller();
void keyboard(unsigned char key, int x, int y);
/*
int main(int argc, char* argv[])
{
  Initialize(argc, argv);

  glutMainLoop();
  
  exit(EXIT_SUCCESS);
}
*/
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
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  ExitOnGLError("ERROR: Could not set OpenGL depth testing options");
  
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);
  ExitOnGLError("ERROR: Could not set OpenGL culling options");
  
  ModelMatrix = IDENTITY_MATRIX;
  ProjectionMatrix = IDENTITY_MATRIX;
  ViewMatrix = IDENTITY_MATRIX;
  TranslateMatrix(&ViewMatrix, 3, 0, -5);
  
  //RotateAboutY(&ViewMatrix, PI/8.);
  create_tuki();
  for(int i=0;i<NUMERO_OGGETTI;i++)
    {
      create_object(i);
    }
  create_ground();
  
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

  glutReshapeFunc(ResizeFunction);
  glutDisplayFunc(RenderFunction);
  glutIdleFunc(main_controller);
  glutTimerFunc(0, TimerFunction, 0);
  glutCloseFunc(DestroyCube);
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
glClearColor(1.,1.,1.,1);

/*gluLookAt(
  0,      0,      1.0,
  0,   0, 1.0,
  0.0,    0.0,    1.0);
*/
ViewMatrix = IDENTITY_MATRIX;
RotateAboutX(&ViewMatrix,spin_x);
RotateAboutY(&ViewMatrix,spin_y);
RotateAboutZ(&ViewMatrix,spin_z);
TranslateMatrix(&ViewMatrix,-(gctr_tuki.pos_x/(double)LUNGHEZZA_CAMMINO), 0, translate_z);
ScaleMatrix(&ViewMatrix,5,5,5);
//RotateAboutX(&ViewMatrix,-PI/8.);
draw_tuki();
  for(int i=0;i<NUMERO_OGGETTI;i++)
    {
      draw_object(i);
    }
  draw_ground();
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

    sprintf(
      TempString,
      "%s: %d Frames Per Second @ %d x %d",
      WINDOW_TITLE_PREFIX,
      FrameCount * 4,
      CurrentWidth,
      CurrentHeight
    );

    glutSetWindowTitle(TempString);
    free(TempString);
  }
  
  FrameCount = 0;
  glutTimerFunc(250, TimerFunction, 1);
}

void create_tuki(void)
{
  const Vertex VERTICES[24] =
    {
      
      { { 16./33.-0.5,19./33.-0.5,0, 1 }, { 0, 0, 1, 1 } },
      { { 9./33.-0.5,18./33.-0.5,0, 1 }, { 1, 0, 0, 1 } },
      { {  20./33.-0.5,18./33.-0.5,0, 1 }, { 0, 0, 0, 1 } },
      { {  12.5/33.-0.5,17./33.-0.5,0, 1 }, { 1, 1, 0, 1 } },
      { { 18./33.-0.5,17./33.-0.5,0, 1 }, { 1, 0, 0, 1 } },
      { { 16./33.-0.5,16./33.-0.5,0, 1 }, { 1, 0, 0, 1 } },
      { {  28./33.-0.5,16./33.-0.5,0, 1 }, { 1, 0, 1, 1 } },
      { {  16.5/33.-0.5,13./33.-0.5,0, 1 }, { 1, 0, 1, 1 } },
      { {  32.9/33.-0.5,13./33.-0.5,0, 1 }, { 0, 0, 1, 1 } },
      { {  10./33.-0.5,12./33.-0.5,0, 1 }, { 0, 0, 1, 1 } },
      { {  28./33.-0.5,11./33.-0.5,0, 1 }, { 0, 0, 1, 1 } },
      { {  9./33.-0.5,10./33.-0.5,0, 1 }, { 0, 0, 1, 1 } },
      { {  17./33.-0.5,10./33.-0.5,0, 1 }, { 0, 0, 1, 1 } },
      { {  32./33.-0.5,10./33.-0.5,0, 1 }, { 0, 0, 1, 1 } },
      { {  13./33.-0.5,5./33.-0.5,0, 1 }, { 0, 0, 1, 1 } },
      { {  0./33.-0.5,4./33.-0.5,0, 1 }, { 0, 0, 1, 1 } },
      { {  10./33.-0.5,4./33.-0.5,0, 1 }, { 0, 0, 1, 1 } },
      { {  12./33.-0.5,4./33.-0.5,0, 1 }, { 0, 0, 1, 1 } },
      { {  14./33.-0.5,4./33.-0.5,0, 1 }, { 0, 0, 1, 1 } },
      { {  13.5/33.-0.5,2./33.-0.5,0, 1 }, { 0, 0, 1, 1 } },
      { {  6./33.-0.5,0./33.-0.5,0, 1 }, { 0, 0, 1, 1 } },
      { {  10./33.-0.5,0./33.-0.5,0, 1 }, { 0, 0, 1, 1 } },
      { {  13./33.-0.50,0./33.-0.5,0, 1 }, { 0, 0, 1, 1 } },
      { {  16./33.-0.5,0./33.-0.5,0, 1 }, { 0, 0, 1, 1 } }
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

void create_object(int i)
{
  const Vertex VERTICES[8] =
    {
      
      { { -.05f, -.05f,  .15f, 1 }, { 1, 0, 0, 1 } },
      { { -.05f,  .05f,  .15f, 1 }, { 1, 0, 0, 1 } },
      { {  .05f,  .05f,  .15f, 1 }, { 1, 0, 0, 1 } },
      { {  .05f, -.05f,  .15f, 1 }, { 1, 0, 0, 1 } },
      { { -.05f, -.05f, .05f, 1 }, { 0, 0, 1, 1 } },
      { { -.05f,  .05f, .05f, 1 }, { 0, 0, 1, 1 } },
      { {  .05f,  .05f, .05f, 1 }, { 0, 0, 1, 1 } },
      { {  .05f, -.05f, .05f, 1 }, { 0, 0, 1, 1 } }
    };

  const GLuint INDICES[36] =
    {
      0,2,1,  0,3,2,
      4,3,0,  4,7,3,
      4,1,5,  4,0,1,
      3,6,2,  3,7,6,
      1,6,5,  1,2,6,
      7,5,6,  7,4,5
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

  glGenBuffers(2, &BufferIds[4+i*2]);
  ExitOnGLError("ERROR: Could not generate the buffer objects");

  glGenVertexArrays(1, &VAO[2+i]);
  ExitOnGLError("ERROR: Could not generate the VAO");
  glBindVertexArray(VAO[2+i]);
  ExitOnGLError("ERROR: Could not bind the VAO");

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  ExitOnGLError("ERROR: Could not enable vertex attributes");

  glBindBuffer(GL_ARRAY_BUFFER, BufferIds[4+i*2]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);
  ExitOnGLError("ERROR: Could not bind the VBO to the VAO");
  
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)0);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)sizeof(VERTICES[0].Position));
  ExitOnGLError("ERROR: Could not set VAO attributes");

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferIds[5+i*2]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(INDICES), INDICES, GL_STATIC_DRAW);
  ExitOnGLError("ERROR: Could not bind the IBO to the VAO");
  
  glBindVertexArray(0);
}


void create_ground(void)
{
  const Vertex VERTICES[4] =
    {
      {{-1,-0.2,-1,1},{0,1,0,1}},
      {{1,-0.2,-1,1},{0,1,0,1}},
      {{1,-0.2,1,1},{0,1,0,1}},
      {{-1,-0.2,1,1},{0,1,0,1}}
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

void DestroyCube(void)
{
  glDetachShader(ShaderIds[0], ShaderIds[1]);
  glDetachShader(ShaderIds[0], ShaderIds[2]);
  glDeleteShader(ShaderIds[1]);
  glDeleteShader(ShaderIds[2]);
  glDeleteProgram(ShaderIds[0]);
  ExitOnGLError("ERROR: Could not destroy the shaders");

  glDeleteBuffers(2, &BufferIds[1]);
  glDeleteVertexArrays(1, &BufferIds[0]);
  ExitOnGLError("ERROR: Could not destroy the buffer objects");
}

void draw_tuki(void)
{
  float CubeAngle;
  
  ModelMatrix = IDENTITY_MATRIX;
  //RotateAboutY(&ModelMatrix, CubeAngle);
  //RotateAboutX(&ModelMatrix, PI);
  ScaleMatrix(&ModelMatrix, 0.05,0.05, 0);
  TranslateMatrix(&ModelMatrix, gctr_tuki.pos_x/(double)LUNGHEZZA_CAMMINO, -0.17, 0);
  
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

void draw_object(int o_index)
{
  float CubeAngle;
  int i = o_index;
  if(!ob[i].exists) return;
    
  ModelMatrix = IDENTITY_MATRIX;
  //RotateAboutY(&ModelMatrix, CubeAngle);
  ScaleMatrix(&ModelMatrix, 0.4,0.4, 0.4);
  TranslateMatrix(&ModelMatrix, ob[i].pos_x/(double)LUNGHEZZA_CAMMINO,-0.2, -0.02);
  
    
  glUseProgram(ShaderIds[0]);
  ExitOnGLError("ERROR: Could not use the shader program, cazzo");
  
  glUniformMatrix4fv(ModelMatrixUniformLocation, 1, GL_FALSE, ModelMatrix.m);
  glUniformMatrix4fv(ViewMatrixUniformLocation, 1, GL_FALSE, ViewMatrix.m);
  ExitOnGLError("ERROR: Could not set the shader uniforms");

  glBindVertexArray(VAO[2+i]);
  ExitOnGLError("ERROR: Could not bind the VAO for drawing purposes");
  
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (GLvoid*)0);
  ExitOnGLError("ERROR: Could not draw the cube");
  
  glBindVertexArray(2+i);
  glUseProgram(0);
  
    
}


void draw_ground(void)
{
  float CubeAngle;

  ModelMatrix = IDENTITY_MATRIX;
  //RotateAboutZ(&ModelMatrix, PI/2.);
  //RotateAboutX(&ModelMatrix, PI/9.);

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

void setSpin(float x, float y, float z)
{
	spin_x = x;
	spin_y = y;
	spin_z = z;
}

void keyboard(unsigned char key, int x, int y)
{

	if(key=='x')
	{
	  spin_x+=0.1;
	  
	  glutPostRedisplay();
	}
	else if(key=='y')
	{
	  spin_y+=0.1;
	  
	  glutPostRedisplay();
	}
	else if(key=='z')
	{
	  spin_z+=0.1;
	  
	  glutPostRedisplay();
	}
	else if(key=='a')
	{
		setSpin(1.0,1.0,1.0);
		glutPostRedisplay();
	}

	else if(key=='i')
	{
		translate_z+=0.01;
		glutPostRedisplay();
	}
	else if(key=='o')
	{
		translate_z-=0.01;
		glutPostRedisplay();
	}



}
