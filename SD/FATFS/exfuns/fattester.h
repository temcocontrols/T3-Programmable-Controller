#ifndef __FATTESTER_H
#define __FATTESTER_H 			   
	   
#include "ff.h"
#include "types.h"

 
uint8 mf_mount(uint8 drv);
uint8 mf_open(uint8*path, uint8 mode);
uint8 mf_close(void);
uint8 mf_read(uint16 len);
uint8 mf_write(uint8*dat, uint16 len);
uint8 mf_opendir(uint8* path);
uint8 mf_readdir(void);
uint8 mf_scan_files(char *path);
uint32 mf_showfree(uint8 *drv);
uint8 mf_lseek(uint32 offset);
uint32 mf_tell(void);
uint32 mf_size(void);
uint8 mf_mkdir(uint8*pname);
uint8 mf_fmkfs(uint8 drv, uint8 mode, uint16 au);
uint8 mf_unlink(uint8 *pname);
uint8 mf_rename(uint8 *oldname, uint8* newname);
void mf_gets(uint16 size);
uint8 mf_putc(uint8 c);
uint8 mf_puts(uint8 *c);
 
#endif





























