#ifndef VDSO_H
#define VDSO_H

#include "types.h"

#define VDSO_ADDR   0x40000000
#define VDSO_MAGIC  0x7664736f  /* 'vdso' */

struct vdso_data {
  uint magic;
  int pid;
};

#endif
