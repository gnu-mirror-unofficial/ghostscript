/* Copyright (C) 2003 Aladdin Enterprises.  All rights reserved.
  
  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License version 2
  as published by the Free Software Foundation.


  This software is provided AS-IS with no warranty, either express or
  implied. That is, this program is distributed in the hope that it will 
  be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  59 Temple Place, Suite 330, Boston, MA, 02111-1307.
 
  For more information about licensing, please refer to
  http://www.ghostscript.com/licensing/. For information on
  commercial licensing, go to http://www.artifex.com/licensing/ or
  contact Artifex Software, Inc., 101 Lucas Valley Road #110,
  San Rafael, CA  94903, U.S.A., +1(415)492-9861.
*/

/* $Id: ttinterp.h,v 1.1 2005/04/18 12:06:00 Arabidopsis Exp $ */

/* Changes after FreeType: cut out the TrueType instruction interpreter. */

/*******************************************************************
 *
 *  ttinterp.h                                              2.2
 *
 *  TrueType bytecode intepreter.
 *
 *  Copyright 1996-1998 by
 *  David Turner, Robert Wilhelm, and Werner Lemberg
 *
 *  This file is part of the FreeType project, and may only be used
 *  modified and distributed under the terms of the FreeType project
 *  license, LICENSE.TXT.  By continuing to use, modify, or distribute
 *  this file you indicate that you have read the license and
 *  understand and accept it fully.
 *
 *
 *  Changes between 2.2 and 2.1:
 *
 *  - a small bugfix in the Push opcodes
 *
 *  Changes between 2.1 and 2.0:
 *
 *  - created the TTExec component to take care of all execution
 *    context management.  The interpreter has now one single
 *    function.
 *
 *  - made some changes to support re-entrancy.  The re-entrant
 *    interpreter is smaller!
 *
 ******************************************************************/

#ifndef TTINTERP_H
#define TTINTERP_H

#include "ttcommon.h"
#include "ttobjs.h"


#ifdef __cplusplus
  extern "C" {
#endif
  
  /* Run instructions in current execution context */
  TT_Error  RunIns( PExecution_Context  exc );
 
#ifdef __cplusplus
  }
#endif

#endif /* TTINTERP_H */


/* END */