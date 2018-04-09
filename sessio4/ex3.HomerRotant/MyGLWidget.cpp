#include "MyGLWidget.h"

#include <iostream>

MyGLWidget::MyGLWidget (QWidget* parent) : QOpenGLWidget(parent)
{
  setFocusPolicy(Qt::ClickFocus);  // per rebre events de teclat
  scale = 1.0f;
}

MyGLWidget::~MyGLWidget ()
{
  if (program != NULL)
    delete program;
}

void MyGLWidget::initializeGL ()
{
  // Cal inicialitzar l'ús de les funcions d'OpenGL
  initializeOpenGLFunctions();  
  glClearColor(0.5, 0.7, 1.0, 1.0); // defineix color de fons (d'esborrat)
  glEnable(GL_DEPTH_TEST);
  angle = 0.0;
  despl = M_PI/4;

  carregaShaders();
  createBuffers();
}

void MyGLWidget::paintGL () 
{
  // Esborrem el frame-buffer
  glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  // Carreguem la transformació de model
  modelTransform();
  modelTransformRotation();
  // Activem el VAO per a pintar Homer 
  glBindVertexArray (VAO_Homer);
  glDrawArrays(GL_TRIANGLES, 0, homer.faces().size()*3);
  glBindVertexArray (0);

  ini_camera();
}

void MyGLWidget::ini_camera() {
  FOV = M_PI/2;
  ra = 1;
  znear = 0.4;
  zfar = 3.0;
  OBS = glm::vec3(0,0,1);
  VRP = glm::vec3(0,0,0);
  UP = glm::vec3(0,1,0);
  
  projectTransform();
  viewTransform();
}

void MyGLWidget::modelTransform () 
{
  // Matriu de transformació de model
  glm::mat4 transform (1.0f);
  transform = glm::scale(transform, glm::vec3(scale));
  glUniformMatrix4fv(transLoc, 1, GL_FALSE, &transform[0][0]);
}

void MyGLWidget::modelTransformRotation() {
  glm::mat4 transform (1.0f);
  transform = glm::translate(transform, glm::vec3(0,0,0));
  transform = glm::scale(transform, glm::vec3(scale));
  transform = glm::rotate(transform, angle, glm::vec3(0.0,1.0,0.0));
  glUniformMatrix4fv(transLoc, 1, GL_FALSE, &transform[0][0]);
}

void MyGLWidget::resizeGL (int w, int h) 
{
  glViewport(0, 0, w, h);
}

void MyGLWidget::keyPressEvent(QKeyEvent* event) 
{
  switch (event->key()) {
    makeCurrent();
    case Qt::Key_S: { // escalar a més gran
      scale += 0.05;
      break;
    }
    case Qt::Key_D: { // escalar a més petit
      scale -= 0.05;
      break;
    }
    case Qt::Key_R: { // escalar a més petit
      angle += despl;
      break;
    }
    default: event->ignore(); break;
  }
  update();	//update crida a paintGL
}

void MyGLWidget::createBuffers () 
{
  //Dades Homer
  homer.load("../models/HomerProves.obj");

  // Creació del Vertex Array Object per pintar
  glGenVertexArrays(1, &VAO_Homer);
  glBindVertexArray(VAO_Homer);

  glGenBuffers(1, &VBO_HomerPos);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_HomerPos);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*homer.faces().size()*3*3, homer.VBO_vertices(), GL_STATIC_DRAW);

  // Activem l'atribut vertexLoc
  glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(vertexLoc);

  glGenBuffers(1, &VBO_HomerCol);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_HomerCol);
  glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*homer.faces().size()*3*3, homer.VBO_matdiff(), GL_STATIC_DRAW);

  // Activem l'atribut colorLoc
  glVertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(colorLoc);

  glBindVertexArray (0);
}

void MyGLWidget::carregaShaders()
{
  // Creem els shaders per al fragment shader i el vertex shader
  QOpenGLShader fs (QOpenGLShader::Fragment, this);
  QOpenGLShader vs (QOpenGLShader::Vertex, this);
  // Carreguem el codi dels fitxers i els compilem
  fs.compileSourceFile("shaders/fragshad.frag");
  vs.compileSourceFile("shaders/vertshad.vert");
  // Creem el program
  program = new QOpenGLShaderProgram(this);
  // Li afegim els shaders corresponents
  program->addShader(&fs);
  program->addShader(&vs);
  // Linkem el program
  program->link();
  // Indiquem que aquest és el program que volem usar
  program->bind();

  // Obtenim identificador per a l'atribut “vertex” del vertex shader
  vertexLoc = glGetAttribLocation (program->programId(), "vertex");
  // Obtenim identificador per a l'atribut “color” del vertex shader
  colorLoc = glGetAttribLocation (program->programId(), "color");
  // Uniform locations
  transLoc = glGetUniformLocation(program->programId(), "TG");
  projLoc = glGetUniformLocation(program->programId(), "proj");
  viewLoc = glGetUniformLocation(program->programId(), "view");
}

void MyGLWidget::projectTransform() {
  glm::mat4 Proj = glm::perspective(FOV, ra, znear, zfar);
  glUniformMatrix4fv (projLoc, 1, GL_FALSE, &Proj[0][0]);
}

void MyGLWidget::viewTransform() {
  glm::mat4 View = glm::lookAt(OBS, VRP, UP);
  glUniformMatrix4fv (viewLoc, 1, GL_FALSE, &View[0][0]);
}
