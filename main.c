#include "piglet.h"
#include <stdio.h>
#include <GLES2/gl2.h>

piglet* p;

void setup() {
  piglet_fullscreen(p, 1);
}

void draw() {
  static float red = 0;
  glClearColor(red, 0.0, 0.0, 0.4);
  glClear(GL_COLOR_BUFFER_BIT);
  glFlush();
  red += 0.01;
  if (red > 1.0) red = 0.0;
}

int main(int argc, char* argv[]) {
  p = piglet_create(setup, draw);
  getchar();
  piglet_destroy(p);
  return 0;
}

