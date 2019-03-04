//======================================================================
// AX11000 Firmware Revision History
// Module Name: MS_TIMER
//
// This document describes the major changes, additions and bug fixes made 
// to the AX11000 Firmware between released versions. 
//======================================================================
version 1.0.3  (2011-06-03)
1. SWTIMER_INTERVAL default setting changes from 50 to 10
2. Modified the msloop count from 11 to 44.
3. Defined the macro, RuntimeCodeAt32KH, in project file respectively,
   and replaced old macro.

version 1.0.2  (2006-11-08)
1. SWTIMER_INTERVAL default setting changes from 200 to 50
2. Modify spin lock to protect variable access.

Version 1.0.1  (2006-05-15)

1.Add RUNTIME_CODE_START_ADDRESS flag to set ExecuteRuntimeFlag to 0 or 1

Version 1.0.0  (2006-04-14)

1. Initial release

