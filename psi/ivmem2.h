/* Copyright (C) 2001-2006 Artifex Software, Inc.
   All Rights Reserved.
  
   This software is provided AS-IS with no warranty, either express or
   implied.

   This software is distributed under license and may not be copied, modified
   or distributed except as expressly authorized under the terms of that
   license.  Refer to licensing information at http://www.artifex.com/
   or contact Artifex Software, Inc.,  7 Mt. Lassen Drive - Suite A-134,
   San Rafael, CA  94903, U.S.A., +1(415)492-9861, for further information.
*/

/* $Id: ivmem2.h,v 1.2 2010/07/10 22:02:44 Arabidopsis Exp $ */
/* VM control user parameter procedures */

#ifndef ivmem2_INCLUDED
#  define ivmem2_INCLUDED

/* Exported by zvmem2.c for zusparam.c */
int set_vm_reclaim(i_ctx_t *, long);
int set_vm_threshold(i_ctx_t *, long);

#endif /* ivmem2_INCLUDED */