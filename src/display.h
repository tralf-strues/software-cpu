#pragma once

static const size_t DISPLAY_DEFAULT_WIDTH  = 640;
static const size_t DISPLAY_DEFAULT_HEIGHT = 480;

static const size_t DISPLAY_PXL_SIZE = 1;

struct Display;

Display* newDisplay           (size_t width, size_t height);
Display* newDisplay           ();
void     deleteDisplay        (Display* display);
void     updateDisplay        (Display* display, unsigned char* buffer);
size_t   getDisplayBufferSize (Display* display);