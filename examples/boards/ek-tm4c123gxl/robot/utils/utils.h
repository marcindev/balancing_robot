//****************************
// Utility functions
// autor: Marcin Gozdziewski
// ***************************

#ifndef UTILS_H
#define UTILS_H

void ZeroBuffer(void* buffer, int len);
void ConfigureUART();
void printBuffer(void* buffer, int len);
void* getTaskHandleByNum(int taskNumber);

#endif // UTILS_H
