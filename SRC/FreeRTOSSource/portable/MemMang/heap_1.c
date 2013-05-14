/*
	FreeRTOS V2.6.0 - Copyright (C) 2003 - 2005 Richard Barry.

	This file is part of the FreeRTOS distribution.

	FreeRTOS is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	FreeRTOS is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with FreeRTOS; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	A special exception to the GPL can be applied should you wish to distribute
	a combined work that includes FreeRTOS, without being obliged to provide
	the source code for any proprietary components.  See the licensing section 
	of http://www.FreeRTOS.org for full details of how and when the exception
	can be applied.

	***************************************************************************
	See http://www.FreeRTOS.org for documentation, latest information, license 
	and contact details.  Please ensure to read the configuration and relevant 
	port sections of the online documentation.
	***************************************************************************
*/

/* 

Changes between V2.5.1 and V2.5.1

	+ The memory pool has been defined within a struct to ensure correct memory
	  alignment on 32bit systems.
*/


/*
 * The simplest possible implementation of pvPortMalloc().  Note that this
 * implementation does NOT allow allocated memory to be freed again.
 *
 * See heap_2.c and heap_3.c for alternative implementations, and the memory
 * management pages of http://www.FreeRTOS.org for more information.
 */
#include <stdlib.h>
#include "projdefs.h"
#include "portable.h"
#include "task.h"

/* Setup the correct byte alignment mask for the defined byte alignment. */
#if portBYTE_ALIGNMENT == 4
	#define heapBYTE_ALIGNMENT_MASK	( ( unsigned portSHORT ) 0x0003 )
#endif

#if portBYTE_ALIGNMENT == 2
	#define heapBYTE_ALIGNMENT_MASK	( ( unsigned portSHORT ) 0x0001 )
#endif

#if portBYTE_ALIGNMENT == 1 
	#define heapBYTE_ALIGNMENT_MASK	( ( unsigned portSHORT ) 0x0000 )
#endif

#ifndef heapBYTE_ALIGNMENT_MASK
	#error "Invalid portBYTE_ALIGNMENT definition"
#endif

/* Allocate the memory for the heap.  The struct is used to force byte
alignment without using any non-portable code. */
static struct xRTOS_HEAP
{
	unsigned portLONG ulDummy;
	unsigned portCHAR ucHeap[ portTOTAL_HEAP_SIZE ];
} xHeap;

static unsigned portSHORT usNextFreeByte = ( unsigned portSHORT ) 0;
/*-----------------------------------------------------------*/

void *pvPortMalloc( unsigned portSHORT usWantedSize ) reentrant
{
void *pvReturn = NULL;

	/* Ensure that blocks are always aligned to the required number of bytes. */
	if( usWantedSize & heapBYTE_ALIGNMENT_MASK )
	{
		/* Byte alignment required. */
		usWantedSize += ( portBYTE_ALIGNMENT - ( usWantedSize & heapBYTE_ALIGNMENT_MASK ) );
	}

	vTaskSuspendAll();
	{
		/* Check there is enough room left for the allocation. */
		if( ( usNextFreeByte + usWantedSize ) < portTOTAL_HEAP_SIZE )
		{
			/* Return the next free byte then increment the index past this
			block. */
			pvReturn = &( xHeap.ucHeap[ usNextFreeByte ] );
			usNextFreeByte += usWantedSize;			
		}	
	}
	cTaskResumeAll();

	return pvReturn;
}
/*-----------------------------------------------------------*/

void vPortFree( void *pv ) reentrant
{
	/* Memory cannot be freed using this scheme.  See heap_2.c and heap_3.c 
	for alternative implementations, and the memory management pages of 
	http://www.FreeRTOS.org for more information. */
	pv = pv;
}
/*-----------------------------------------------------------*/

void vPortInitialiseBlocks( void ) reentrant
{
	/* Only required when static memory is not cleared. */
	usNextFreeByte = ( unsigned portSHORT ) 0;
}


