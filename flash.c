#include "piglet.h"
#include <stdio.h>
#include <GLES2/gl2.h>
int shouldflash = 0;
piglet* p;
void setup() {
    piglet_fullscreen(p, 1);
}
void draw() {
    if (shouldflash)
      glClearColor(1, 1, 1, 1);
    else
      glClearColor(0, 0, 0, 1);
    shouldflash = 0;
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();
}
int main(int argc, char* argv[]) {
    p = piglet_create(setup, draw);
    unsigned impulseCount = 0;
    char c;
    do {
        c = getchar();
        shouldflash = 1;
        impulseCount++;
        printf("impulses: %u", impulseCount);
    } while (c==10);
    piglet_destroy(p);
    return 0;
}
