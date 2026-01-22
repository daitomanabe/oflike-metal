// stb_image - v2.28 - public domain image loader
// Minimal subset for PNG, JPG, BMP loading
// Full version at: https://github.com/nothings/stb

#ifndef STBI_INCLUDE_STB_IMAGE_H
#define STBI_INCLUDE_STB_IMAGE_H

#ifndef STBI_NO_STDIO
#include <stdio.h>
#endif

#define STBI_VERSION 1

enum {
  STBI_default = 0,
  STBI_grey = 1,
  STBI_grey_alpha = 2,
  STBI_rgb = 3,
  STBI_rgb_alpha = 4
};

typedef unsigned char stbi_uc;
typedef unsigned short stbi_us;

#ifdef __cplusplus
extern "C" {
#endif

// Primary API - load image from file/memory
#ifndef STBI_NO_STDIO
stbi_uc *stbi_load(char const *filename, int *x, int *y, int *channels_in_file, int desired_channels);
#endif

stbi_uc *stbi_load_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels);

void stbi_image_free(void *retval_from_stbi_load);

// Get reason for failure
const char *stbi_failure_reason(void);

#ifdef __cplusplus
}
#endif

#endif // STBI_INCLUDE_STB_IMAGE_H

#ifdef STB_IMAGE_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <math.h>

#ifndef STBI_ASSERT
#include <assert.h>
#define STBI_ASSERT(x) assert(x)
#endif

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef signed short int16;
typedef unsigned int uint32;
typedef signed int int32;

// Fast abs
#ifdef _MSC_VER
#define stbi_abs(x) abs(x)
#else
#define stbi_abs(x) ((x) < 0 ? -(x) : (x))
#endif

static char *stbi__g_failure_reason;

const char *stbi_failure_reason(void) {
  return stbi__g_failure_reason;
}

static int stbi__err(const char *str) {
  stbi__g_failure_reason = (char *)str;
  return 0;
}

#define stbi__errpuc(x,y)  ((unsigned char *)(size_t) (stbi__err(x) ? NULL : NULL))

static void *stbi__malloc(size_t size) {
  return malloc(size);
}

void stbi_image_free(void *retval_from_stbi_load) {
  free(retval_from_stbi_load);
}

// I/O context
typedef struct {
  uint32 img_x, img_y;
  int img_n, img_out_n;
  const uint8 *buffer_start;
  const uint8 *buffer;
  const uint8 *buffer_end;
} stbi__context;

static void stbi__start_mem(stbi__context *s, const uint8 *buffer, int len) {
  s->buffer_start = buffer;
  s->buffer = buffer;
  s->buffer_end = buffer + len;
}

static int stbi__at_eof(stbi__context *s) {
  return s->buffer >= s->buffer_end;
}

static uint8 stbi__get8(stbi__context *s) {
  if (s->buffer < s->buffer_end)
    return *s->buffer++;
  return 0;
}

static int stbi__get16be(stbi__context *s) {
  int z = stbi__get8(s);
  return (z << 8) + stbi__get8(s);
}

static uint32 stbi__get32be(stbi__context *s) {
  uint32 z = stbi__get16be(s);
  return (z << 16) + stbi__get16be(s);
}

static int stbi__get16le(stbi__context *s) {
  int z = stbi__get8(s);
  return z + (stbi__get8(s) << 8);
}

static uint32 stbi__get32le(stbi__context *s) {
  uint32 z = stbi__get16le(s);
  return z + (stbi__get16le(s) << 16);
}

static void stbi__skip(stbi__context *s, int n) {
  if (n < 0) {
    s->buffer = s->buffer_end;
    return;
  }
  s->buffer += n;
}

static int stbi__getn(stbi__context *s, uint8 *buffer, int n) {
  if (s->buffer + n > s->buffer_end)
    n = (int)(s->buffer_end - s->buffer);
  memcpy(buffer, s->buffer, n);
  s->buffer += n;
  return n;
}

// BMP loader
static int stbi__bmp_test_raw(stbi__context *s) {
  int r;
  int sz;
  if (stbi__get8(s) != 'B') return 0;
  if (stbi__get8(s) != 'M') return 0;
  stbi__get32le(s); // file size
  stbi__get16le(s); // reserved
  stbi__get16le(s); // reserved
  stbi__get32le(s); // offset
  sz = stbi__get32le(s);
  r = (sz == 12 || sz == 40 || sz == 56 || sz == 108 || sz == 124);
  return r;
}

static int stbi__bmp_test(stbi__context *s) {
  const uint8 *start = s->buffer;
  int r = stbi__bmp_test_raw(s);
  s->buffer = start;
  return r;
}

static uint8 *stbi__bmp_load(stbi__context *s, int *x, int *y, int *comp, int req_comp) {
  uint8 *out;
  unsigned int mr = 0, mg = 0, mb = 0, ma = 0;
  int psize = 0, i, j, compress = 0, width;
  int bpp, flip_vertically, pad, target, offset, hsz;

  if (stbi__get8(s) != 'B' || stbi__get8(s) != 'M') return stbi__errpuc("not BMP", "Corrupt BMP");
  stbi__get32le(s); // file size
  stbi__get16le(s); // reserved
  stbi__get16le(s); // reserved
  offset = stbi__get32le(s);
  hsz = stbi__get32le(s);

  if (hsz != 12 && hsz != 40 && hsz != 56 && hsz != 108 && hsz != 124)
    return stbi__errpuc("unknown BMP", "BMP type not supported");

  if (hsz == 12) {
    s->img_x = stbi__get16le(s);
    s->img_y = stbi__get16le(s);
  } else {
    s->img_x = stbi__get32le(s);
    s->img_y = stbi__get32le(s);
  }
  if (stbi__get16le(s) != 1) return stbi__errpuc("bad BMP", "bad BMP");
  bpp = stbi__get16le(s);

  if (bpp == 1) return stbi__errpuc("monochrome", "BMP type not supported");

  flip_vertically = ((int)s->img_y) > 0;
  s->img_y = stbi_abs((int)s->img_y);

  if (hsz == 12) {
    if (bpp < 24) psize = (offset - 14 - 24) / 3;
  } else {
    compress = stbi__get32le(s);
    if (compress == 1 || compress == 2) return stbi__errpuc("BMP RLE", "BMP RLE not supported");
    stbi__get32le(s); // discard sizeof
    stbi__get32le(s); // discard hres
    stbi__get32le(s); // discard vres
    stbi__get32le(s); // discard colorsused
    stbi__get32le(s); // discard max important
    if (hsz == 40 || hsz == 56) {
      if (hsz == 56) {
        stbi__get32le(s);
        stbi__get32le(s);
        stbi__get32le(s);
        stbi__get32le(s);
      }
      if (bpp == 16 || bpp == 32) {
        mr = mg = mb = 0;
        if (compress == 0) {
          if (bpp == 32) {
            mr = 0xffu << 16;
            mg = 0xffu << 8;
            mb = 0xffu << 0;
            ma = 0xffu << 24;
          } else {
            mr = 31u << 10;
            mg = 31u << 5;
            mb = 31u << 0;
          }
        } else if (compress == 3) {
          mr = stbi__get32le(s);
          mg = stbi__get32le(s);
          mb = stbi__get32le(s);
        }
      }
    } else {
      STBI_ASSERT(hsz == 108 || hsz == 124);
      mr = stbi__get32le(s);
      mg = stbi__get32le(s);
      mb = stbi__get32le(s);
      ma = stbi__get32le(s);
      stbi__get32le(s); // discard color space
      for (i = 0; i < 12; ++i)
        stbi__get32le(s); // discard color space parameters
      if (hsz == 124) {
        stbi__get32le(s); // discard intent
        stbi__get32le(s); // discard offset
        stbi__get32le(s); // discard size
        stbi__get32le(s); // discard reserved
      }
    }
    if (bpp < 16) psize = (offset - 14 - hsz) >> 2;
  }

  s->img_n = ma ? 4 : 3;
  if (req_comp && req_comp >= 3)
    target = req_comp;
  else
    target = s->img_n;

  *x = s->img_x;
  *y = s->img_y;
  if (comp) *comp = s->img_n;

  out = (uint8 *)stbi__malloc(target * s->img_x * s->img_y);
  if (!out) return stbi__errpuc("outofmem", "Out of memory");

  if (bpp < 16) {
    uint8 pal[256][4];
    int z = 0;
    if (psize == 0 || psize > 256) { free(out); return stbi__errpuc("invalid", "Corrupt BMP"); }
    for (i = 0; i < psize; ++i) {
      pal[i][2] = stbi__get8(s);
      pal[i][1] = stbi__get8(s);
      pal[i][0] = stbi__get8(s);
      if (hsz != 12) stbi__get8(s);
      pal[i][3] = 255;
    }
    stbi__skip(s, offset - 14 - hsz - psize * (hsz == 12 ? 3 : 4));
    if (bpp == 4) width = (s->img_x + 1) >> 1;
    else if (bpp == 8) width = s->img_x;
    else { free(out); return stbi__errpuc("bad bpp", "Corrupt BMP"); }
    pad = (-width) & 3;
    for (j = 0; j < (int)s->img_y; ++j) {
      for (i = 0; i < (int)s->img_x; i += (bpp == 8 ? 1 : 2)) {
        int v = stbi__get8(s), v2 = 0;
        if (bpp == 4) { v2 = v & 15; v >>= 4; }
        out[z++] = pal[v][0];
        out[z++] = pal[v][1];
        out[z++] = pal[v][2];
        if (target == 4) out[z++] = 255;
        if (i + 1 < (int)s->img_x) {
          int v3 = (bpp == 8) ? stbi__get8(s) : v2;
          out[z++] = pal[v3][0];
          out[z++] = pal[v3][1];
          out[z++] = pal[v3][2];
          if (target == 4) out[z++] = 255;
        }
      }
      stbi__skip(s, pad);
    }
  } else {
    int rshift = 0, gshift = 0, bshift = 0, ashift = 0, rcount = 0, gcount = 0, bcount = 0, acount = 0;
    int z = 0;
    int easy = 0;

    stbi__skip(s, offset - 14 - hsz);

    if (bpp == 24) width = 3 * s->img_x;
    else if (bpp == 16) width = 2 * s->img_x;
    else width = 0;
    pad = (-width) & 3;

    if (bpp == 24) easy = 1;
    else if (bpp == 32) {
      if (mb == 0xff && mg == 0xff00 && mr == 0x00ff0000 && ma == 0xff000000) easy = 2;
    }

    if (!easy) {
      if (!mr || !mg || !mb) { free(out); return stbi__errpuc("bad masks", "Corrupt BMP"); }
      rshift = stbi__high_bit(mr) - 7; rcount = stbi__bitcount(mr);
      gshift = stbi__high_bit(mg) - 7; gcount = stbi__bitcount(mg);
      bshift = stbi__high_bit(mb) - 7; bcount = stbi__bitcount(mb);
      ashift = stbi__high_bit(ma) - 7; acount = stbi__bitcount(ma);
    }

    for (j = 0; j < (int)s->img_y; ++j) {
      if (easy) {
        for (i = 0; i < (int)s->img_x; ++i) {
          unsigned char a;
          out[z + 2] = stbi__get8(s);
          out[z + 1] = stbi__get8(s);
          out[z + 0] = stbi__get8(s);
          z += 3;
          a = (easy == 2 ? stbi__get8(s) : 255);
          if (target == 4) out[z++] = a;
        }
      } else {
        for (i = 0; i < (int)s->img_x; ++i) {
          uint32 v = (bpp == 16 ? (uint32)stbi__get16le(s) : stbi__get32le(s));
          out[z++] = (uint8)stbi__shiftsigned(v & mr, rshift, rcount);
          out[z++] = (uint8)stbi__shiftsigned(v & mg, gshift, gcount);
          out[z++] = (uint8)stbi__shiftsigned(v & mb, bshift, bcount);
          if (target == 4)
            out[z++] = (ma ? (uint8)stbi__shiftsigned(v & ma, ashift, acount) : 255);
        }
      }
      stbi__skip(s, pad);
    }
  }

  if (flip_vertically) {
    uint8 t;
    for (j = 0; j < (int)s->img_y >> 1; ++j) {
      uint8 *p1 = out + j * s->img_x * target;
      uint8 *p2 = out + (s->img_y - 1 - j) * s->img_x * target;
      for (i = 0; i < (int)s->img_x * target; ++i) {
        t = p1[i]; p1[i] = p2[i]; p2[i] = t;
      }
    }
  }

  return out;
}

// Bit manipulation helpers
static int stbi__high_bit(unsigned int z) {
  int n = 0;
  if (z == 0) return -1;
  if (z >= 0x10000) { n += 16; z >>= 16; }
  if (z >= 0x00100) { n += 8; z >>= 8; }
  if (z >= 0x00010) { n += 4; z >>= 4; }
  if (z >= 0x00004) { n += 2; z >>= 2; }
  if (z >= 0x00002) { n += 1; }
  return n;
}

static int stbi__bitcount(unsigned int a) {
  a = (a & 0x55555555) + ((a >> 1) & 0x55555555);
  a = (a & 0x33333333) + ((a >> 2) & 0x33333333);
  a = (a + (a >> 4)) & 0x0f0f0f0f;
  a = (a + (a >> 8));
  a = (a + (a >> 16));
  return a & 0xff;
}

static int stbi__shiftsigned(int v, int shift, int bits) {
  int result;
  int z = 0;
  if (shift < 0) v <<= -shift;
  else v >>= shift;
  result = v;
  z = bits;
  while (z < 8) {
    result += v >> z;
    z += bits;
  }
  return result;
}

// PNG loader (simplified)
static int stbi__check_png_header(stbi__context *s) {
  static uint8 png_sig[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };
  int i;
  for (i = 0; i < 8; ++i)
    if (stbi__get8(s) != png_sig[i]) return stbi__err("bad png sig", "Not a PNG");
  return 1;
}

// For this minimal implementation, we'll use macOS APIs for PNG/JPG
// This is a stub that returns NULL - actual implementation should use platform APIs

static uint8 *stbi__png_load(stbi__context *s, int *x, int *y, int *comp, int req_comp) {
  (void)s; (void)x; (void)y; (void)comp; (void)req_comp;
  return stbi__errpuc("png not implemented", "Use platform image loading");
}

static int stbi__png_test(stbi__context *s) {
  static uint8 png_sig[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };
  int i;
  for (i = 0; i < 8; ++i)
    if (stbi__get8(s) != png_sig[i]) { s->buffer -= i + 1; return 0; }
  s->buffer -= 8;
  return 1;
}

// JPG test
static int stbi__jpeg_test(stbi__context *s) {
  int r;
  r = stbi__get8(s) == 0xff && stbi__get8(s) == 0xd8;
  s->buffer -= 2;
  return r;
}

// Main entry point
stbi_uc *stbi_load_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels) {
  stbi__context s;
  stbi__start_mem(&s, buffer, len);

  if (stbi__bmp_test(&s))
    return stbi__bmp_load(&s, x, y, channels_in_file, desired_channels);

  // For PNG/JPG, return NULL with error - should use platform APIs
  if (stbi__png_test(&s) || stbi__jpeg_test(&s)) {
    return stbi__errpuc("use platform loader", "PNG/JPG: use NSImage/CGImage on macOS");
  }

  return stbi__errpuc("unknown format", "Image not recognized");
}

#ifndef STBI_NO_STDIO
stbi_uc *stbi_load(char const *filename, int *x, int *y, int *comp, int req_comp) {
  FILE *f = fopen(filename, "rb");
  unsigned char *result;
  if (!f) return stbi__errpuc("can't open", "Unable to open file");
  fseek(f, 0, SEEK_END);
  long len = ftell(f);
  fseek(f, 0, SEEK_SET);
  unsigned char *buffer = (unsigned char *)malloc(len);
  if (!buffer) { fclose(f); return stbi__errpuc("outofmem", "Out of memory"); }
  fread(buffer, 1, len, f);
  fclose(f);
  result = stbi_load_from_memory(buffer, (int)len, x, y, comp, req_comp);
  free(buffer);
  return result;
}
#endif

#endif // STB_IMAGE_IMPLEMENTATION
