#include <stdint.h>     /* for standard integer types uint8_t etc. */
#include <stdbool.h>    /* for the standard bool type. */

/*
struct sockaddr	{
		unsigned short sa_familiy;
		char sa_data[14];
	};
struct in_addr {
	union {
		struct {u_char s_b1,s_b2,s_b3,s_b4}S_un_b;
		struct {u_short s_w1,s_w2}S_un_w;
		ulong S_addr;
	}S_un;
	#define s_addr S_un.S_addr
};


struct sockaddr_in {
		short int sin_famili;
		unsigned short int sin_port;
		struct in_addr sin_addr;
		unsigned char sin_zero[8];
	};
*/

#ifndef FD_SETSIZE 
#define FD_SETSIZE 64 
#endif /* FD_SETSIZE */ 

typedef struct fd_set { 
	uint16_t fd_count; /* how many are SET? */ 
	SOCKET fd_array[FD_SETSIZE]; /* an array of SOCKETs */ 
} fd_set; 

extern int PASCAL FAR __WSAFDIsSet(SOCKET, fd_set FAR *); 

#define FD_CLR(fd, set) do { \ 
	uint16_t __i; \ 
	for (__i = 0; __i < ((fd_set FAR *)(set))->;fd_count ; __i++) { \ 
	if (((fd_set FAR *)(set))->;fd_array[__i] == fd) { \ 
	while (__i < ((fd_set FAR *)(set))->;fd_count-1) { \ 
	((fd_set FAR *)(set))->;fd_array[__i] = \ 
	((fd_set FAR *)(set))->;fd_array[__i+1]; \ 
	__i++; \ 
} \ 
	((fd_set FAR *)(set))->;fd_count--; \ 
break; \ 
} \ 
} \ 
} while(0) 

#define FD_SET(fd, set) do { \ 
uint16_t __i; \ 
for (__i = 0; __i < ((fd_set FAR *)(set))->;fd_count; __i++) { \ 
if (((fd_set FAR *)(set))->;fd_array[__i] == (fd)) { \ 
break; \ 
} \ 
} \ 
if (__i == ((fd_set FAR *)(set))->;fd_count) { \ 
if (((fd_set FAR *)(set))->;fd_count < FD_SETSIZE) { \ 
((fd_set FAR *)(set))->;fd_array[__i] = (fd); \ 
((fd_set FAR *)(set))->;fd_count++; \ 
} \ 
} \ 
} while(0) 

#define FD_ZERO(set) (((fd_set FAR *)(set))->;fd_count=0) 

#define FD_ISSET(fd, set) __WSAFDIsSet((SOCKET)(fd), (fd_set FAR *)(set)) 


 int select(int maxfd,fd_set *rdset,fd_set *wrset,fd_set *exset,struct timeval *timeout); 
