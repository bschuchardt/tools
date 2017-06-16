#ifndef EMXP_H
#define EMXP_H
/*=========================================================================
 * Copyright (C) Servio Corp. 1991-1994.  All Rights Reserved.
 *
 * Name - 
 *
 * Purpose -
 *
 * $Id: EmxP.h,v 30.4 1994/01/29 02:05:07 marcs Exp $
 *
 *========================================================================*/

#if defined(MOTIF12)
#include <Xm/PrimitiveP.h>
#endif

typedef struct
    {
    int	    dummy;
    } EmxClassPart;


typedef struct _EmxClassRec
    {
    CoreClassPart	core_class;
#ifdef MOTIF
    XmPrimitiveClassPart primitive_class;
#endif
    EmxClassPart	emx_class;
    } EmxClassRec;


typedef struct _EmxRec
    {
    CorePart		core;
#ifdef MOTIF
    XmPrimitivePart	primitive;
#endif
    EmxVars		emx;
    } EmxRec;

#endif /* EMXP_H */
