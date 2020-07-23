#include "Utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

//gcc -o c4 chapter.4.1.c Utils.c -lglut -lGL -lGLU -lGLEW -lm

#define WINDOW_TITLE_PREFIX "Chapter 1"

int
  CurrentWidth = 800,
  CurrentHeight = 600,
  WindowHandle = 0;
unsigned FrameCount = 0;
GLuint
  ProjectionMatrixUniformLocation,
  ViewMatrixUniformLocation,
  ModelMatrixUniformLocation,
  VAO[3]={ 0 },
  BufferIds[6] = { 0 },
  ShaderIds[3] = { 0 };

Matrix
  ProjectionMatrix,
  ViewMatrix,
  ModelMatrix;

float CubeRotation = 0;
float CubePosition = 0;
clock_t LastTime = 0;



void Initialize(int, char*[]);
void InitWindow(int, char*[]);
void ResizeFunction(int, int);
void RenderFunction(void);
void TimerFunction(int);
void IdleFunction(void);
void CreateCube(void);
void CreateSecondCube(void);
void CreateGround(void);
void DestroyCube(void);
void DrawCube(void);
void DrawSecondCube(void);
void DrawGround(void);
int main_controller();
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
  TranslateMatrix(&ViewMatrix, 0, 0, -5);
  RotateAboutY(&ViewMatrix, PI/8.);
  CreateCube();
  CreateSecondCube();
  CreateGround();
  
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

  /*gluLookAt(
	    0,      0,      1.0,
	    0,   0, 1.0,
	    0.0,    0.0,    1.0);
  */
  ViewMatrix = IDENTITY_MATRIX;
  TranslateMatrix(&ViewMatrix,-CubePosition, 0, -2);
  DrawCube();
  DrawSecondCube();
  DrawGround();
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

void CreateCube(void)
{
  const Vertex VERTICES[8] =
    {
      
      { { -.05f, -.05f,  .15f, 1 }, { 0, 0, 1, 1 } },
      { { -.05f,  .05f,  .15f, 1 }, { 1, 0, 0, 1 } },
      { {  .05f,  .05f,  .15f, 1 }, { 0, 1, 0, 1 } },
      { {  .05f, -.05f,  .15f, 1 }, { 1, 1, 0, 1 } },
      { { -.05f, -.05f, .05f, 1 }, { 1, 1, 1, 1 } },
      { { -.05f,  .05f, .05f, 1 }, { 1, 0, 0, 1 } },
      { {  .05f,  .05f, .05f, 1 }, { 1, 0, 1, 1 } },
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

void CreateSecondCube(void)
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


void CreateGround(void)
{
  const Vertex VERTICES[4] =
    {
      {{-1,-1,0,1},{0,1,0,1}},
      {{1,-1,0,1},{0,1,0,1}},
      {{1,1,0,1},{0,1,0,1}},
      {{-1,1,0,1},{0,1,0,1}}
    };
  
  const GLuint INDICES[6] =
    {
      0,1,2, 2,3,0
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

void DrawCube(void)
{
  float CubeAngle;
  clock_t Now = clock();
  if (LastTime == 0)
    LastTime = Now;
  CubePosition += 0.001;
  CubeRotation += 10.0f * ((float)(Now - LastTime) /1000.);
  CubeAngle = DegreesToRadians(CubeRotation);
  LastTime = Now;

  ModelMatrix = IDENTITY_MATRIX;
  RotateAboutY(&ModelMatrix, CubeAngle);
  RotateAboutX(&ModelMatrix, CubeAngle);
  TranslateMatrix(&ModelMatrix, CubePosition, 0, 0);
  
  glUseProgram(ShaderIds[0]);
  ExitOnGLError("ERROR: Could not use the shader program, cazzo");
  
  glUniformMatrix4fv(ModelMatrixUniformLocation, 1, GL_FALSE, ModelMatrix.m);
  glUniformMatrix4fv(ViewMatrixUniformLocation, 1, GL_FALSE, ViewMatrix.m);
  ExitOnGLError("ERROR: Could not set the shader uniforms");

  glBindVertexArray(VAO[0]);
  ExitOnGLError("ERROR: Could not bind the VAO for drawing purposes");
  
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (GLvoid*)0);
  ExitOnGLError("ERROR: Could not draw the cube");
  
  glBindVertexArray(0);
  glUseProgram(0);
      
}

void DrawSecondCube(void)
{
  float CubeAngle;
  clock_t Now = clock();
  if (LastTime == 0)
    LastTime = Now;
  
  CubeRotation += 10.0f * ((float)(Now - LastTime) /1000.);
  CubeAngle = DegreesToRadians(CubeRotation);
  LastTime = Now;

  ModelMatrix = IDENTITY_MATRIX;
  //RotateAboutY(&ModelMatrix, CubeAngle);
  TranslateMatrix(&ModelMatrix, 0.5, 0, 0.15);
    
  glUseProgram(ShaderIds[0]);
  ExitOnGLError("ERROR: Could not use the shader program, cazzo");
  
  glUniformMatrix4fv(ModelMatrixUniformLocation, 1, GL_FALSE, ModelMatrix.m);
  glUniformMatrix4fv(ViewMatrixUniformLocation, 1, GL_FALSE, ViewMatrix.m);
  ExitOnGLError("ERROR: Could not set the shader uniforms");

  glBindVertexArray(VAO[2]);
  ExitOnGLError("ERROR: Could not bind the VAO for drawing purposes");
  
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (GLvoid*)0);
  ExitOnGLError("ERROR: Could not draw the cube");
  
  glBindVertexArray(2);
  glUseProgram(0);
  
    
}


void DrawGround(void)
{
  float CubeAngle;
  clock_t Now = clock();
  if (LastTime == 0)
    LastTime = Now;
  
  CubeRotation += 10.0f * ((float)(Now - LastTime) /1000.);
  CubeAngle = DegreesToRadians(CubeRotation);
  LastTime = Now;

  ModelMatrix = IDENTITY_MATRIX;
  //RotateAboutY(&ModelMatrix, CubeAngle);
  RotateAboutX(&ModelMatrix, PI/3.);

  glUseProgram(ShaderIds[0]);
  ExitOnGLError("ERROR: Could not use the shader program, cazzo");
  
  glUniformMatrix4fv(ModelMatrixUniformLocation, 1, GL_FALSE, ModelMatrix.m);
  glUniformMatrix4fv(ViewMatrixUniformLocation, 1, GL_FALSE, ViewMatrix.m);
  ExitOnGLError("ERROR: Could not set the shader uniforms");

  glBindVertexArray(VAO[1]);
  ExitOnGLError("ERROR: Could not bind the VAO for drawing purposes");
  
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (GLvoid*)0);
  ExitOnGLError("ERROR: Could not draw the Ground");
  
  glBindVertexArray(1);
  glUseProgram(0);
  
    
}