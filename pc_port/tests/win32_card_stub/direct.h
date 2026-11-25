#ifndef PE_TEST_WIN32_DIRECT_H
#define PE_TEST_WIN32_DIRECT_H
#include <sys/stat.h>
static inline int _mkdir(const char *path) { return mkdir(path, 0755); }
#endif
