#include "piglet.h"
#include <GLES2/gl2.h>
#include <stdio.h>

void setup() {
  printf("setup()\n");
}

void draw() {
  static float f = 0;
  static unsigned n = 0;

  glClearColor(f, 0.0, 0.0, 0.4); // use alpha to see see other contexts
  glClear(GL_COLOR_BUFFER_BIT);
  glFlush();

  if (n % 60 == 0)
    printf("draw()\n");
  n++;

  f += 0.01f;
  if (f > 1.0f)
    f = 0.0f;
}

int main(int argc, char* argv[]) {
  piglet* p = piglet_create_detail(setup, draw, 1000, 800, 100, 100, 0);
  getchar();
  piglet_set_rect(p, 200, 200, 300, 300);
  getchar();
  piglet_fullscreen(p, 1);
  getchar();
  piglet_fullscreen(p, 0);
  getchar();
  piglet_set_rect(p, 300, 300, 100, 100);
  getchar();
  piglet_fullscreen(p, 1);
  getchar();
  piglet_destroy(p);
  return 0;
}
