#ifndef UTIL_H
#define UTIL_H

#include <assert.h>
//* 所有文件都可能会使用到的头文件 杂项
//* 今后可能会将该文件拆分成更详细的多个文件

#define NON_COPYABLE(name)     \
  name(const name &) = delete; \
  name &operator=(const name &) = delete;

#endif