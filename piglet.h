#ifndef __PIGLET__
#define __PIGLET__

struct piglet_;
typedef struct piglet_ piglet;

piglet* piglet_create(void (*setup)(void), void (*draw)(void));

piglet* piglet_create_detail(
  void (*setup)(void),
  void (*draw)(void),
  unsigned x,
  unsigned y,
  unsigned width,
  unsigned height,
  unsigned fullscreen);

void piglet_fullscreen(piglet* p, unsigned fullscreen);
void piglet_set_rect(piglet* p, unsigned x, unsigned y, unsigned width, unsigned height);
void piglet_destroy(piglet* p);

#endif
