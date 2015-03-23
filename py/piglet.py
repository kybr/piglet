from cffi import FFI
ffi = FFI()
ffi.cdef("""
typedef void* piglet;
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
""")

lib = ffi.dlopen("../piglet.so");

def Create(setup, draw):
  return lib.piglet_create(ffi.callback("void(void)", setup), ffi.callback("void(void)", draw))

def Destroy(piglet):
  lib.piglet_destroy(piglet)
