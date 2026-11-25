#ifndef PE_TEST_WIN32_WINDOWS_H
#define PE_TEST_WIN32_WINDOWS_H
#define MOVEFILE_REPLACE_EXISTING 0x1u
int MoveFileExA(const char *source, const char *destination, unsigned long flags);
unsigned long GetLastError(void);
#endif
