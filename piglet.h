#ifndef __PIGLET__
#define __PIGLET__

struct piglet_;
typedef struct piglet_ piglet;

extern piglet* piglet_create(void (*setup)(void), void (*draw)(void));

extern piglet* piglet_create_detail(
  void (*setup)(void),
  void (*draw)(void),
  unsigned x,
  unsigned y,
  unsigned width,
  unsigned height,
  unsigned fullscreen);

extern void piglet_fullscreen(piglet* p, unsigned fullscreen);
extern void piglet_set_rect(piglet* p, unsigned x, unsigned y, unsigned width, unsigned height);
extern void piglet_destroy(piglet* p);

#endif
