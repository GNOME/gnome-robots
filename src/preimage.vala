/*
 * Copyright 2020 Andrey Kutejko <andy128k@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * For more details see the file COPYING.
 */

using Cairo;
using Gdk;
using Rsvg;

public class GamesPreimage {
    private int width = 0;
    private int height = 0;

    private Handle? rsvg_handle = null;
    private FontOptions? font_options;

    /* raster pixbuf data */
    private Pixbuf? pixbuf;

    private bool scalable = false;

    public GamesPreimage () {}

    public GamesPreimage.from_file (string filename) throws GLib.Error {
        try {
            rsvg_handle = new Handle.from_file (filename);
        } catch (GLib.Error e) {
            // ignore
        }

        if (rsvg_handle != null) {
            scalable = true;

            var data = rsvg_handle.get_dimensions ();

            if (data.width == 0 || data.height == 0) {
                throw new PixbufError.FAILED ("Image has zero extent");
            }

            width = data.width;
            height = data.height;
        } else {
            /* Not an SVG */
            scalable = false;

            pixbuf = new Pixbuf.from_file (filename);
            width = pixbuf.get_width ();
            height = pixbuf.get_height ();
        }
    }

    /**
     * Turns on antialising of preimage, if it contains an SVG image.
     */
    public void set_font_options (owned FontOptions? font_options) {
        this.font_options = (owned) font_options;
    }

    /**
     * @width: the desired width
     * @height: the desired height
     *
     * Creates a #GdkPixbuf at the specified @width and @height.
     **/
    public Pixbuf render (int width, int height)
        requires (width > 0 && height > 0)
    {
        if (scalable) {
            /* Render vector image */
            return render_sub (null,
                               width,
                               height,
                               0.0, 0.0,
                               ((double) width) / ((double) this.width),
                               ((double) height) / ((double) this.height));
        } else {
            /* Render raster image */
            return pixbuf.scale_simple (width, height, InterpType.BILINEAR);
        }
    }

    /**
     * @width: the desired width
     * @height: the desired height
     *
     * Renders from @preimage's image at the specified
     * @width and @height to @cr.
     **/
    private void render_cairo (Context cr, int width, int height)
        requires (width > 0 && height > 0)
    {
        if (scalable) {
            /* Render vector image */
            render_cairo_sub (cr,
                              null,
                              width,
                              height,
                              0.0, 0.0,
                              ((double) width) / ((double) this.width),
                              ((double) height) / ((double) this.height));
        } else {
            /* FIXMEchpe: we don't really need this fallback anymore */
            /* Render raster image */
            var scaled_pixbuf = pixbuf.scale_simple (width, height, InterpType.BILINEAR);

            cr.save ();
            cairo_set_source_pixbuf (cr, scaled_pixbuf, 0, 0);
            cr.paint ();
            cr.restore ();
        }
    }

    /**
     * @node: a SVG node ID (starting with "#"), or %NULL
     * @width: the width of the clip region
     * @height: the height of the clip region
     * @xoffset: the x offset of the clip region
     * @yoffset: the y offset of the clip region
     * @xzoom: the x zoom factor
     * @yzoom: the y zoom factor
     *
     * Creates a #GdkPixbuf with the dimensions @width by @height,
     * and renders the subimage of @preimage specified by @node to it,
     * transformed by @xzoom, @yzoom and offset by @xoffset and @yoffset,
     * clipped to @width and @height.
     * If @node is NULL, the whole image is rendered into tha clip region.
     *
     * Returns: (transfer full) (allow-none): a new #GdkPixbuf, or %NULL if there was an error or @preimage
     * isn't a scalable SVG image
     */
    private Pixbuf? render_sub (string? node,
                               int width, int height,
                               double xoffset, double yoffset,
                               double xzoom, double yzoom) {
        if (!scalable)
            return null;

        var surface = new ImageSurface (Format.ARGB32, width, height);
        var cr = new Context (surface);
        render_cairo_sub (cr, node, width, height, xoffset, yoffset, xzoom, yzoom);
        return Gdk.pixbuf_get_from_surface (surface, 0, 0, width, height);
    }

    /**
     * @node: a SVG node ID (starting with "#"), or %NULL
     * @width: the width of the clip region
     * @height: the height of the clip region
     * @xoffset: the x offset of the clip region
     * @yoffset: the y offset of the clip region
     * @xzoom: the x zoom factor
     * @yzoom: the y zoom factor
     **/
    private void render_cairo_sub (Context cr,
                                  string? node,
                                  int width,
                                  int height,
                                  double xoffset,
                                  double yoffset,
                                  double xzoom,
                                  double yzoom) {
        if (!scalable)
            return;

        if (font_options != null) {
            cr.set_antialias (font_options.get_antialias ());
            cr.set_font_options (font_options);
        }

        Cairo.Matrix matrix = Cairo.Matrix.identity ();
        matrix.scale (xzoom, yzoom);
        matrix.translate (xoffset, yoffset);

        cr.set_matrix (matrix);

        rsvg_handle.render_cairo_sub (cr, node);
    }

    /**
     * Returns true iff preimage contains an SVG image
     */
    public bool is_scalable () {
        return scalable;
    }

    /**
     * natural width of the image
     */
    public int get_width () {
        return width;
    }

    /**
     * natural height of the image
     */
    public int get_height () {
        return height;
    }
}

