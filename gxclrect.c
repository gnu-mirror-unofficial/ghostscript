/* Copyright (C) 1997 Aladdin Enterprises.  All rights reserved.
  
  This file is part of GNU Ghostscript.
  
  GNU Ghostscript is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY.  No author or distributor accepts responsibility to
  anyone for the consequences of using it or for whether it serves any
  particular purpose or works at all, unless he says so in writing.  Refer to
  the GNU General Public License for full details.
  
  Everyone is granted permission to copy, modify and redistribute GNU
  Ghostscript, but only under the conditions described in the GNU General
  Public License.  A copy of this license is supposed to have been given to
  you along with GNU Ghostscript so you can know your rights and
  responsibilities.  It should be in a file named COPYING.  Among other
  things, the copyright notice and this notice must be preserved on all
  copies.
  
  Aladdin Enterprises is not affiliated with the Free Software Foundation or
  the GNU Project.  GNU Ghostscript, as distributed by Aladdin Enterprises,
  does not depend on any other GNU software.
*/

/* gxclrect.c */
/* Rectangle-oriented command writing for command list */
#include "gx.h"
#include "gserrors.h"
#include "gsutil.h"			/* for gs_next_ids */
#include "gxdevice.h"
#include "gxdevmem.h"			/* must precede gxcldev.h */
#include "gxcldev.h"

#define cdev cwdev

/* ---------------- Writing utilities ---------------- */

#define cmd_set_rect(rect)\
  ((rect).x = x, (rect).y = y,\
   (rect).width = width, (rect).height = height)

/* Write a rectangle. */
private int
cmd_size_rect(register const gx_cmd_rect *prect)
{	return
	  cmd_sizew(prect->x) + cmd_sizew(prect->y) +
	  cmd_sizew(prect->width) + cmd_sizew(prect->height);
}
private byte *
cmd_put_rect(register const gx_cmd_rect *prect, register byte *dp)
{	cmd_putw(prect->x, dp);
	cmd_putw(prect->y, dp);
	cmd_putw(prect->width, dp);
	cmd_putw(prect->height, dp);
	return dp;
}

int
cmd_write_rect_cmd(gx_device_clist_writer *cldev, gx_clist_state *pcls,
  int op, int x, int y, int width, int height)
{	int dx = x - pcls->rect.x;
	int dy = y - pcls->rect.y;
	int dwidth = width - pcls->rect.width;
	int dheight = height - pcls->rect.height;
#define check_range_xy(rmin, rmax)\
  ((unsigned)(dx - rmin) <= (rmax - rmin) &&\
   (unsigned)(dy - rmin) <= (rmax - rmin))
#define check_range_w(rmin, rmax)\
  ((unsigned)(dwidth - rmin) <= (rmax - rmin))
#define check_ranges(rmin, rmax)\
  (check_range_xy(rmin, rmax) && check_range_w(rmin, rmax) &&\
   (unsigned)(dheight - rmin) <= (rmax - rmin))
	cmd_set_rect(pcls->rect);
	if ( dheight == 0 && check_range_w(cmd_min_dw_tiny, cmd_max_dw_tiny) &&
	     check_range_xy(cmd_min_dxy_tiny, cmd_max_dxy_tiny)
	   )
	  {	byte op_tiny = op + 0x20 + dwidth - cmd_min_dw_tiny;
		byte *dp;

		if ( dx == width - dwidth && dy == 0 )
		  { set_cmd_put_op(dp, cldev, pcls, op_tiny + 8, 1);
		  }
		else
		  { set_cmd_put_op(dp, cldev, pcls, op_tiny, 2);
		    dp[1] = (dx << 4) + dy - (cmd_min_dxy_tiny * 0x11);
		  }
	  }
#define rmin cmd_min_short
#define rmax cmd_max_short
	else if ( check_ranges(rmin, rmax) )
	   {	int dh = dheight - cmd_min_dxy_tiny;
		byte *dp;
		if ( (unsigned)dh <= cmd_max_dxy_tiny - cmd_min_dxy_tiny &&
		     dh != 0 && dy == 0
		   )
		   {	op += dh;
			set_cmd_put_op(dp, cldev, pcls, op + 0x10, 3);
			if_debug3('L', "    rs2:%d,%d,0,%d\n",
				  dx, dwidth, dheight);
		   }
		else
		   {	set_cmd_put_op(dp, cldev, pcls, op + 0x10, 5);
			if_debug4('L', "    rs4:%d,%d,%d,%d\n",
				  dx, dwidth, dy, dheight);
			dp[3] = dy - rmin;
			dp[4] = dheight - rmin;
		   }
		dp[1] = dx - rmin;
		dp[2] = dwidth - rmin;
	   }
#undef rmin
#undef rmax
	else if ( dy >= -2 && dy <= 1 && dheight >= -2 && dheight <= 1 &&
		  (dy + dheight) != -4
		)
	  {	byte *dp;
		int rcsize = 1 + cmd_sizew(x) + cmd_sizew(width);
		set_cmd_put_op(dp, cldev, pcls,
			       op + ((dy + 2) << 2) + dheight + 2, rcsize);
		++dp;
		cmd_put2w(x, width, dp);
	  }
	else
	   {	byte *dp;
		int rcsize = 1 + cmd_size_rect(&pcls->rect);
		set_cmd_put_op(dp, cldev, pcls, op, rcsize);
		if_debug5('L', "    r%d:%d,%d,%d,%d\n",
			  rcsize - 1, dx, dwidth, dy, dheight);
		cmd_put_rect(&pcls->rect, dp + 1);
	   }
	return 0;
}

/* ---------------- Driver procedures ---------------- */

int
clist_fill_rectangle(gx_device *dev, int x, int y, int width, int height,
  gx_color_index color)
{	fit_fill(dev, x, y, width, height);
	BEGIN_RECT
	cmd_disable_lop(cdev, pcls);
	if ( color != pcls->colors[1] )
	  {	int code = cmd_put_color(cdev, pcls, &clist_select_color1,
					 color, &pcls->colors[1]);
		if ( code < 0 )
		  return code;
	  }
	{ int code = cmd_write_rect_cmd(cdev, pcls, cmd_op_fill_rect, x, y,
					width, height);
	  if ( code < 0 )
	    return code;
	}
	END_RECT
	return 0;
}

int
clist_strip_tile_rectangle(gx_device *dev, const gx_strip_bitmap *tile,
  int x, int y, int width, int height,
  gx_color_index color0, gx_color_index color1, int px, int py)
{	int depth =
	  (color1 == gx_no_color_index && color0 == gx_no_color_index ?
	   dev->color_info.depth : 1);

	fit_fill(dev, x, y, width, height);
	BEGIN_RECT
	ulong offset_temp;

	cmd_disable_lop(cdev, pcls);
	if ( !cls_has_tile_id(cdev, pcls, tile->id, offset_temp) )
	   {	if ( tile->id == gx_no_bitmap_id ||
		     clist_change_tile(cdev, pcls, tile, depth) < 0
		   )
		  {	int code =
			  gx_default_strip_tile_rectangle(dev, tile,
						x, y, width, height,
						color0, color1, px, py);
			if ( code < 0 )
			  return code;
			goto endr;
		  }
	   }
	if ( color0 != pcls->tile_colors[0] || color1 != pcls->tile_colors[1] )
	  {	int code = cmd_set_tile_colors(cdev, pcls, color0, color1);
		if ( code < 0 )
		  return code;
	  }
	if ( px != pcls->tile_phase.x || py != pcls->tile_phase.y )
	  {	int code = cmd_set_tile_phase(cdev, pcls, px, py);
		if ( code < 0 )
		  return code;
	  }
	{ int code = cmd_write_rect_cmd(cdev, pcls, cmd_op_tile_rect, x, y,
					width, height);
	  if ( code < 0 )
	    return code;
	}
endr:	;
	END_RECT
	return 0;
}

int
clist_copy_mono(gx_device *dev,
    const byte *data, int data_x, int raster, gx_bitmap_id id,
    int x, int y, int width, int height,
    gx_color_index color0, gx_color_index color1)
{	int y0;
	gx_bitmap_id orig_id = id;

	fit_copy(dev, data, data_x, raster, id, x, y, width, height);
	y0 = y;
	BEGIN_RECT
	int dx = data_x & 7;
	int w1 = dx + width;
	const byte *row = data + (y - y0) * raster + (data_x >> 3);
	int code;

	cmd_disable_lop(cdev, pcls);
	cmd_disable_clip(cdev, pcls);
	if ( color0 != pcls->colors[0] )
	  {	code = cmd_set_color0(cdev, pcls, color0);
		if ( code < 0 )
		  return code;
	  }
	if ( color1 != pcls->colors[1] )
	  {	code = cmd_set_color1(cdev, pcls, color1);
		if ( code < 0 )
		  return code;
	  }
	/* Don't bother to check for a possible cache hit: */
	/* tile_rectangle and fill_mask handle those cases. */
copy:	{	gx_cmd_rect rect;
		int rsize;
		byte op = (byte)cmd_op_copy_mono;
		byte *dp;
		uint csize;
		int code;

		rect.x = x, rect.y = y;
		rect.width = w1, rect.height = height;
		rsize = (dx ? 3 : 1) + cmd_size_rect(&rect);
		code = cmd_put_bits(cdev, pcls, row, w1, height, raster,
				    rsize, (orig_id == gx_no_bitmap_id ?
					    1 << cmd_compress_rle :
					    cmd_mask_compress_any),
				    &dp, &csize);
		if ( code < 0 )
		  { if ( code != gs_error_limitcheck )
		      return code;
		    /* The bitmap was too large; split up the transfer. */
		    if ( height > 1 )
		      {	/* Split the transfer by reducing the height.
			 * See the comment above BEGIN_RECT in gxcldev.h.
			 */
			height >>= 1;
			goto copy;
		      }
		    {	/* Split a single (very long) row. */
			int w2 = w1 >> 1;
			code = clist_copy_mono(dev, row, dx + w2,
				raster, gx_no_bitmap_id, x + w2, y,
				w1 - w2, 1, color0, color1);
			if ( code < 0 )
			  return code;
			w1 = w2;
			goto copy;
		    }
		  }
		op += code;
		if ( dx )
		  { *dp++ = cmd_count_op(cmd_opv_set_misc, 2);
		    *dp++ = cmd_set_misc_data_x + dx;
		  }
		*dp++ = cmd_count_op(op, csize);
		cmd_put2w(x, y, dp);
		cmd_put2w(w1, height, dp);
		pcls->rect = rect;
	   }
	END_RECT
	return 0;
}

int
clist_copy_color(gx_device *dev,
  const byte *data, int data_x, int raster, gx_bitmap_id id,
  int x, int y, int width, int height)
{	int depth = dev->color_info.depth;
	int y0;
	int data_x_bit;

	fit_copy(dev, data, data_x, raster, id, x, y, width, height);
	y0 = y;
	data_x_bit = data_x * depth;
	BEGIN_RECT
	int dx = (data_x_bit & 7) / depth;
	int w1 = dx + width;
	const byte *row = data + (y - y0) * raster + (data_x_bit >> 3);
	int code;

	cmd_disable_lop(cdev, pcls);
	cmd_disable_clip(cdev, pcls);
	if ( pcls->color_is_alpha )
	  { byte *dp;
	    set_cmd_put_op(dp, cdev, pcls, cmd_opv_set_copy_color, 1);
	    pcls->color_is_alpha = 0;
	  }
copy:	  {	gx_cmd_rect rect;
		int rsize;
		byte op = (byte)cmd_op_copy_color_alpha;
		byte *dp;
		uint csize;

		rect.x = x, rect.y = y;
		rect.width = w1, rect.height = height;
		rsize = (dx ? 3 : 1) + cmd_size_rect(&rect);
		code = cmd_put_bits(cdev, pcls, row, w1 * depth,
				    height, raster, rsize,
				    1 << cmd_compress_rle, &dp, &csize);
		if ( code < 0 )
		  { if ( code != gs_error_limitcheck )
		      return code;
		    /* The bitmap was too large; split up the transfer. */
		    if ( height > 1 )
		      {	/* Split the transfer by reducing the height.
			 * See the comment above BEGIN_RECT in gxcldev.h.
			 */
			height >>= 1;
			goto copy;
		      }
		    {	/* Split a single (very long) row. */
			int w2 = w1 >> 1;
			code = clist_copy_color(dev, row, dx + w2,
				raster, gx_no_bitmap_id, x + w2, y,
				w1 - w2, 1);
			if ( code < 0 )
			  return code;
			w1 = w2;
			goto copy;
		    }
		  }
		op += code;
		if ( dx )
		  { *dp++ = cmd_count_op(cmd_opv_set_misc, 2);
		    *dp++ = cmd_set_misc_data_x + dx;
		  }
		*dp++ = cmd_count_op(op, csize);
		cmd_put2w(x, y, dp);
		cmd_put2w(w1, height, dp);
		pcls->rect = rect;

	  }
	END_RECT
	return 0;
}

int
clist_copy_alpha(gx_device *dev, const byte *data, int data_x,
  int raster, gx_bitmap_id id, int x, int y, int width, int height,
  gx_color_index color, int depth)
{	/* I don't like copying the entire body of clist_copy_color */
	/* just to change 2 arguments and 1 opcode, */
	/* but I don't see any alternative that doesn't require */
	/* another level of procedure call even in the common case. */
	int log2_depth = depth >> 1;	/* works for 1,2,4 */
	int y0;
	int data_x_bit;

	fit_copy(dev, data, data_x, raster, id, x, y, width, height);
	y0 = y;
	data_x_bit = data_x << log2_depth;
	BEGIN_RECT
	int dx = (data_x_bit & 7) >> log2_depth;
	int w1 = dx + width;
	const byte *row = data + (y - y0) * raster + (data_x_bit >> 3);
	int code;

	cmd_disable_lop(cdev, pcls);
	cmd_disable_clip(cdev, pcls);
	if ( !pcls->color_is_alpha )
	  { byte *dp;
	    set_cmd_put_op(dp, cdev, pcls, cmd_opv_set_copy_alpha, 1);
	    pcls->color_is_alpha = 1;
	  }
	if ( color != pcls->colors[1] )
	  {	int code = cmd_set_color1(cdev, pcls, color);
		if ( code < 0 )
		  return code;
	  }
copy:	  {	gx_cmd_rect rect;
		int rsize;
		byte op = (byte)cmd_op_copy_color_alpha;
		byte *dp;
		uint csize;

		rect.x = x, rect.y = y;
		rect.width = w1, rect.height = height;
		rsize = (dx ? 4 : 2) + cmd_size_rect(&rect);
		code = cmd_put_bits(cdev, pcls, row, w1 << log2_depth,
				    height, raster, rsize,
				    1 << cmd_compress_rle, &dp, &csize);
		if ( code < 0 )
		  { if ( code != gs_error_limitcheck )
		      return code;
		    /* The bitmap was too large; split up the transfer. */
		    if ( height > 1 )
		      {	/* Split the transfer by reducing the height.
			 * See the comment above BEGIN_RECT in gxcldev.h.
			 */
			height >>= 1;
			goto copy;
		      }
		    {	/* Split a single (very long) row. */
			int w2 = w1 >> 1;
			code = clist_copy_alpha(dev, row, dx + w2,
				raster, gx_no_bitmap_id, x + w2, y,
				w1 - w2, 1, color, depth);
			if ( code < 0 )
			  return code;
			w1 = w2;
			goto copy;
		    }
		  }
		op += code;
		if ( dx )
		  { *dp++ = cmd_count_op(cmd_opv_set_misc, 2);
		    *dp++ = cmd_set_misc_data_x + dx;
		  }
		*dp++ = cmd_count_op(op, csize);
		*dp++ = depth;
		cmd_put2w(x, y, dp);
		cmd_put2w(w1, height, dp);
		pcls->rect = rect;
	  }
	END_RECT
	return 0;
}

int
clist_strip_copy_rop(gx_device *dev,
  const byte *sdata, int sourcex, uint sraster, gx_bitmap_id id,
  const gx_color_index *scolors,
  const gx_strip_bitmap *textures, const gx_color_index *tcolors,
  int x, int y, int width, int height,
  int phase_x, int phase_y, gs_logical_operation_t lop)
{	gs_rop3_t rop = lop_rop(lop);
	gx_strip_bitmap tile_with_id;
	const gx_strip_bitmap *tiles = textures;
	int y0;

	if ( scolors != 0 && scolors[0] != scolors[1] )
	  { fit_fill(dev, x, y, width, height);
	  }
	else
	  { fit_copy(dev, sdata, sourcex, sraster, id, x, y, width, height);
	  }
	y0 = y;
	/*
	 * We shouldn't need to put the logic below inside BEGIN/END_RECT,
	 * but the lop_enabled flags are per-band.
	 */
	BEGIN_RECT
	const byte *row = sdata + (y - y0) * sraster;
	int code;

	if ( rop3_uses_T(rop) )
	  { if ( tcolors == 0 || tcolors[0] != tcolors[1] )
	      { ulong offset_temp;
	        if ( !cls_has_tile_id(cdev, pcls, tiles->id, offset_temp) )
		  { /* Change tile.  If there is no id, generate one. */
		    if ( tiles->id == gx_no_bitmap_id )
		      { tile_with_id = *tiles;
		        tile_with_id.id = gs_next_ids(1);
			tiles = &tile_with_id;
		      }
		    code = clist_change_tile(cdev, pcls, tiles,
					     (tcolors != 0 ? 1 :
					      dev->color_info.depth));
		    if ( code < 0 )
		      { /*
			 * If the error is a limitcheck, we have a tile that
			 * is too big to fit in the command reading buffer.
			 * For now, just divide up the transfer into scan
			 * lines.  (If a single scan line won't fit, punt.)
			 * Eventually, we'll need a way to transfer the tile
			 * in pieces.
			 */
			uint rep_height = tiles->rep_height;
			gs_id ids;
			gx_strip_bitmap line_tile;
			int iy;

			if ( code != gs_error_limitcheck ||
			     rep_height == 1 ||
			     /****** CAN'T HANDLE SHIFT YET ******/
			     tiles->rep_shift != 0
			   )
			  return code;
			/*
			 * Allocate enough fake IDs, since the inner call on
			 * clist_strip_copy_rop will need them anyway.
			 */
			ids = gs_next_ids(min(height, rep_height));
			line_tile = *tiles;
			line_tile.size.y = 1;
			line_tile.rep_height = 1;
			for ( iy = 0; iy < height; ++iy )
			  { line_tile.data = tiles->data + line_tile.raster *
			      ((y + iy + phase_y) % rep_height);
			    line_tile.id = ids + (iy % rep_height);
			    /*
			     * Note that since we're only transferring
			     * a single scan line, phase_y is irrelevant;
			     * we may as well use the current tile phase
			     * so we don't have to write extra commands.
			     */
			    code = clist_strip_copy_rop(dev,
				(sdata == 0 ? 0 : row + iy * sraster),
				sourcex, sraster,
				gx_no_bitmap_id, scolors, &line_tile, tcolors,
				x, y + iy, width, 1,
				phase_x, pcls->tile_phase.y, lop);
			    if ( code < 0 )
			      return code;
			  }
			continue;
		      }
		    if ( phase_x != pcls->tile_phase.x ||
			 phase_y != pcls->tile_phase.y
		       )
		      { code = cmd_set_tile_phase(cdev, pcls, phase_x,
						  phase_y);
		        if ( code < 0 )
			  return code;
		      }
		  }
	      }
	    /* Set the tile colors. */
	    code = (tcolors != 0 ?
		    cmd_set_tile_colors(cdev, pcls, tcolors[0], tcolors[1]) :
		    cmd_set_tile_colors(cdev, pcls, gx_no_color_index,
					gx_no_color_index));
	    if ( code < 0 )
	      return code;
	  }
	if ( lop != pcls->lop )
	  { code = cmd_set_lop(cdev, pcls, lop);
	    if ( code < 0 )
	      return code;
	  }
	cmd_enable_lop(cdev, pcls);
	/* Set lop_enabled to -1 so that fill_rectangle / copy_* */
	/* won't attempt to set it to 0. */
	pcls->lop_enabled = -1;
	if ( scolors != 0 )
	  { if ( scolors[0] == scolors[1] )
	      code = clist_fill_rectangle(dev, x, y, width, height,
					  scolors[1]);
	    else
	      code = clist_copy_mono(dev, row, sourcex, sraster, id,
				     x, y, width, height,
				     scolors[0], scolors[1]);
	  }
	else
	  code = clist_copy_color(dev, row, sourcex, sraster, id,
				  x, y, width, height);
	pcls->lop_enabled = 1;
	if ( code < 0 )
	  return code;
	END_RECT
	return 0;	  
}
