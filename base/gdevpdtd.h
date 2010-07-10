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

/* $Id: gdevpdtd.h,v 1.2 2010/07/10 22:02:28 Arabidopsis Exp $ */
/* FontDescriptor structure and API for pdfwrite */

#ifndef gdevpdtd_INCLUDED
#  define gdevpdtd_INCLUDED

#include "gdevpdtx.h"
#include "gdevpdtb.h"

/* ================ Types and structures ================ */

/*
 * FontDescriptors are handled as pseudo-resources.  Multiple Font resources
 * may share a descriptor.  We don't need to use reference counting to keep
 * track of this, since all descriptors persist until the device is closed.
 * The CharSet entry in the FontDescriptor for a Type 1 subset font lists
 * the glyphs that are included in the subset, so the FontDescriptor cannot
 * be written until the font has been written, but we could use an indirect
 * object for the CharSet and write the FontDescriptor itself early.
 * However, we don't think that is worthwhile, since FontDescriptors are
 * small objects compared to the fonts themselves, and it's simpler to keep
 * all the FontDescriptors until the end.
 *
 * Note that FontDescriptors and BaseFonts correspond 1-to-1.  While the PDF
 * specification allows multiple FontDescriptors for a single BaseFont, this
 * has no value: all the information in the FontDescriptor is derived from
 * the BaseFont, so all the FontDescriptors for the same BaseFont must be
 * the same.
 */
/*
 * Font names in PDF files have caused an enormous amount of trouble, so we
 * document specifically how they are handled in each structure.
 *
 * The PDF Reference says that the FontName in a font descriptor must be
 * the same as the BaseFont in the font or CIDFont resource(s) that
 * reference(s) it.
 *
 * We never create a font descriptor without also creating a font resource
 * that references it, so we set the FontName at the same time as the
 * BaseFont of the font resource.  For more information, see gdevpdtf.h.
 */

#ifndef pdf_font_descriptor_DEFINED
#  define pdf_font_descriptor_DEFINED
typedef struct pdf_font_descriptor_s pdf_font_descriptor_t;
#endif

/* ================ Procedures ================ */

/*
 * Allocate a FontDescriptor, initializing the FontType and rid from the
 * gs_font.
 */
int pdf_font_descriptor_alloc(gx_device_pdf *pdev,
			      pdf_font_descriptor_t **ppfd,
			      gs_font_base *font, bool embed);

/*
 * Get the object ID of a FontDescriptor.
 */
long pdf_font_descriptor_id(const pdf_font_descriptor_t *pfd);

/*
 * Get the FontType of a FontDescriptor.
 */
font_type pdf_font_descriptor_FontType(const pdf_font_descriptor_t *pfd);

/*
 * Get the embedding status of a FontDescriptor.
 */
bool pdf_font_descriptor_embedding(const pdf_font_descriptor_t *pfd);

/*
 * Check for subset font.
 */
bool pdf_font_descriptor_is_subset(const pdf_font_descriptor_t *pfd);

/*
 * Return a reference to the FontName of a FontDescriptor, similar to
 * pdf_base_font_name.
 */
gs_string *pdf_font_descriptor_name(pdf_font_descriptor_t *pfd);

/*
 * Return the (copied, subset or complete) font associated with a FontDescriptor.
 * This procedure probably shouldn't exist....
 */
gs_font_base *pdf_font_descriptor_font(const pdf_font_descriptor_t *pfd, bool complete);

/*
 * Drop the copied complete font associated with a FontDescriptor.
 */
void pdf_font_descriptor_drop_complete_font(const pdf_font_descriptor_t *pfd);

/*
 * Return a reference to the name of a FontDescriptor's base font, per
 * pdf_base_font_name.
 */
gs_string *pdf_font_descriptor_base_name(const pdf_font_descriptor_t *pfd);

/*
 * Copy a glyph from a font to the stable copy.  Return 0 if this is a
 * new glyph, 1 if it was already copied.
 */
int pdf_font_used_glyph(pdf_font_descriptor_t *pfd, gs_glyph glyph,
			gs_font_base *font);

/*
 * Compute the FontDescriptor metrics for a font.
 */
int pdf_compute_font_descriptor(gx_device_pdf *pdev, pdf_font_descriptor_t *pfd);

/*
 * Finish a FontDescriptor by computing the metric values, and then
 * writing the associated embedded font if any.
 */
int pdf_finish_FontDescriptor(gx_device_pdf *pdev,
			      pdf_resource_t *pfd);

int pdf_finish_resources(gx_device_pdf *pdev, pdf_resource_type_t type,
			int (*finish_proc)(gx_device_pdf *,
					   pdf_resource_t *));
/*
 * Write a FontDescriptor.
 */
int pdf_write_FontDescriptor(gx_device_pdf *pdev,
			     pdf_resource_t *pfd);

/*
 * Release a FontDescriptor components.
 */
int pdf_release_FontDescriptor_components(gx_device_pdf *pdev, pdf_resource_t *pfd);

/*
 * Mark a FontDescriptor used in a text.
 */
int pdf_mark_font_descriptor_used(gx_device_pdf *pdev, pdf_font_descriptor_t *pfd);

/*
 * Mark a FontDescriptor Flags value as symbolic
 */
int mark_font_descriptor_symbolic(pdf_font_resource_t *pdfont);

#endif /* gdevpdtd_INCLUDED */