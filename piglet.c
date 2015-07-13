#include "piglet.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <bcm_host.h>

#include <stdlib.h>
#include <stdio.h>

typedef struct piglet_ {
  void (*setup)(void);
  void (*draw)(void);
  EGLDisplay display;
  EGLSurface surface;
  EGLContext context;
  uint32_t displayWidth, displayHeight;
  EGL_DISPMANX_WINDOW_T window;
  DISPMANX_DISPLAY_HANDLE_T dispman_display;
  VC_RECT_T windowRect, fullscreenRect;
  unsigned fullscreen;
  unsigned firstTime;
} piglet;

piglet* singleton = NULL; // only allow a single window per process for now

void vsync_happened(DISPMANX_UPDATE_HANDLE_T ignore, void * arg) {

  piglet* p = (piglet*)arg;

  if (p == 0) {
    printf("ERROR: piglet was null\n");
    return;
  }

  if (eglMakeCurrent(p->display, p->surface, p->surface, p->context) != EGL_TRUE) {
    printf("FAIL: eglMakeCurrent\n");
    exit(1);
  }

  if (p->firstTime) {
    p->firstTime = 0;
    p->setup();
  }
  else {
    p->draw();
  }

  eglSwapBuffers(p->display, p->surface);
}

piglet* piglet_create(void (*setup)(void), void (*draw)(void)) {

  if (singleton != NULL) {
    printf("ERROR: piglet already running. call piglet_create only once, ever\n");
    return singleton;
  }

  bcm_host_deinit(); // XXX need this?
  bcm_host_init();

  uint32_t width, height;
  if (graphics_get_display_size(0, &width, &height) < 0) {
    printf("FAIL: graphics_get_display_size\n");
    exit(1);
  }

  //bcm_host_deinit(); // XXX need this?

  return piglet_create_detail(setup, draw, width - 200, 0, 200, 200, 0);
}

piglet* piglet_create_detail(
  void (*setup)(void),
  void (*draw)(void),
  unsigned x,
  unsigned y,
  unsigned width,
  unsigned height,
  unsigned fullscreen) {

  if (singleton != NULL) {
    printf("ERROR: piglet already running. call piglet_create only once, ever\n");
    return singleton;
  }

  piglet* p = malloc(sizeof(piglet));
  singleton = p;

  memset(p, 0, sizeof(piglet));

  p->windowRect = (VC_RECT_T){x, y, width, height};
  p->window.width = width;
  p->window.height = height;
  p->fullscreen = fullscreen;
  p->setup = setup;
  p->draw = draw;
  p->firstTime = 1;

  bcm_host_deinit(); // XXX need this?
  bcm_host_init();

  if (graphics_get_display_size(0, &p->displayWidth, &p->displayHeight) < 0) {
    printf("FAIL: graphics_get_display_size\n");
    exit(1);
  }

  //printf("displayWidth:%u, displayHeight:%u\n", p->displayWidth, p->displayHeight);
  p->fullscreenRect = (VC_RECT_T){0, 0, p->displayWidth, p->displayHeight};

  if ((p->dispman_display = vc_dispmanx_display_open(0 /* LCD */)) <= 0) {
    printf("FAIL: vc_dispmanx_display_open\n");
    exit(1);
  }

  // set initial/default window size and position - FULLSCREEN
  //
  VC_RECT_T* rect = fullscreen ? &p->fullscreenRect : &p->windowRect;
  p->window.width = fullscreen ? p->fullscreenRect.width : p->windowRect.width;
  p->window.height = fullscreen ? p->fullscreenRect.height : p->windowRect.height;
  VC_RECT_T sourceRectangle = {0, 0, p->window.width << 16, p->window.height << 16};

  // create a dispmanx "element"
  //
  DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(0);
  p->window.element = vc_dispmanx_element_add(
    update,
    p->dispman_display,
    0, // layer. -128 would be behind the framebuffer
    rect,
    0, // DISPMANX_RESOURCE_HANDLE_T source
    &sourceRectangle,
    DISPMANX_PROTECTION_NONE,
    0, // alpha
    0, // clamp?
    0);// transform
  vc_dispmanx_update_submit_sync(update);

  ////////////////////////////////////////////////////////////
  // egl stuff
  //

  p->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  if (eglInitialize(p->display, NULL, NULL) != EGL_TRUE) {
    printf("FAIL: eglInitialize\n");
    exit(1);
  }

  if (eglBindAPI(EGL_OPENGL_ES_API) != EGL_TRUE) {
    printf("FAIL: eglBindAPI\n");
    exit(1);
  }

  EGLConfig config;
  EGLint num_config;

  static const EGLint attribute_list[] = {
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_NONE};

  static const EGLint context_attributes[] = {
    EGL_CONTEXT_CLIENT_VERSION, 2,
    EGL_NONE};

  if (eglChooseConfig(p->display, attribute_list, &config, 1, &num_config) != EGL_TRUE) {
    printf("FAIL: eglChooseConfig\n");
    exit(1);
  }

  if ((p->context = eglCreateContext(p->display, config, EGL_NO_CONTEXT, context_attributes)) == EGL_NO_CONTEXT) {
    printf("FAIL: eglCreateContext\n");
    exit(1);
  }

  if ((p->surface = eglCreateWindowSurface(p->display, config, &p->window, NULL)) == EGL_NO_SURFACE) {
    printf("FAIL: eglCreateWindowSurface\n");
    exit(1);
  }

  // register callback on vsync
  //
  if (vc_dispmanx_vsync_callback(
        p->dispman_display,
        (DISPMANX_CALLBACK_FUNC_T)vsync_happened,
        p /* XXX pass this piglet as the callback arguments - seems broked */) != 0)
  {
    printf("FAIL: vc_dispmanx_vsync_callback\n");
    exit(1);
  }

  return p;
}

void piglet_fullscreen(piglet* p, unsigned fullscreen) {
  if (p->fullscreen != fullscreen) {
    VC_RECT_T* rect = fullscreen ? &p->fullscreenRect : &p->windowRect;
    p->window.width = fullscreen ? p->fullscreenRect.width : p->windowRect.width;
    p->window.height = fullscreen ? p->fullscreenRect.height : p->windowRect.height;
    VC_RECT_T sourceRectangle = {0, 0, p->window.width << 16, p->window.height << 16};

    DISPMANX_UPDATE_HANDLE_T t = vc_dispmanx_update_start(0);
    vc_dispmanx_element_change_attributes(
      t,
      p->window.element,
      0, // change flags
      0, // layer
      0, // opacity
      rect, // this changes the change flags under the covers
      &sourceRectangle,
      0, // DISPMANX_RESOURCE_HANDLE_T mask
      0);// DISPMANX_TRANSFORM_T transform
    vc_dispmanx_update_submit_sync(t);

    p->fullscreen = fullscreen;
  }
}

void piglet_set_rect(piglet* p, unsigned x, unsigned y, unsigned width, unsigned height) {
  if (p->fullscreen)
    p->fullscreen = 0;

  p->windowRect = (VC_RECT_T){x, y, width, height};
  p->window.width = p->windowRect.width;
  p->window.height = p->windowRect.height;
  VC_RECT_T sourceRectangle = {0, 0, p->window.width << 16, p->window.height << 16};

  DISPMANX_UPDATE_HANDLE_T t = vc_dispmanx_update_start(0);
  vc_dispmanx_element_change_attributes(
    t,
    p->window.element,
    0, // change flags
    0, // layer
    0, // opacity
    &p->windowRect, // this changes the change flags under the covers
    &sourceRectangle,
    0, // DISPMANX_RESOURCE_HANDLE_T mask
    0);// DISPMANX_TRANSFORM_T transform
  vc_dispmanx_update_submit_sync(t);
}

void piglet_destroy(piglet* p) {
  free(p);
  bcm_host_deinit();
}
