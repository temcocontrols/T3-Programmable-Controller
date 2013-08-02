#ifndef FUNC_LINUX_H
#define FUNC_LINUX_H

#include <stdint.h>
#include <stddef.h>



uint16_t htons(uint16_t port);
char *getenv(char *envvar); 
uint16_t ntohs(uint16_t netshort); 



#endif