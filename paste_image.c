
#include "purple.h"
#include "gtkplugin.h"
#include "gtkconv.h"
#include "gtkimhtml.h"
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#define PLUGIN_ID "eionrobb_paste_image"

gsize
gdk_pixbuf_get_byte_length (const GdkPixbuf *pixbuf)
{
	g_return_val_if_fail (GDK_IS_PIXBUF (pixbuf), -1);

        return ((gdk_pixbuf_get_height(pixbuf) - 1) * gdk_pixbuf_get_rowstride(pixbuf) +
                gdk_pixbuf_get_width(pixbuf) * ((gdk_pixbuf_get_n_channels(pixbuf) * gdk_pixbuf_get_bits_per_sample(pixbuf) + 7) / 8));
}

guchar *
gdk_pixbuf_get_pixels_with_length (const GdkPixbuf *pixbuf,
                                   guint           *length)
{
	g_return_val_if_fail (GDK_IS_PIXBUF (pixbuf), NULL);

        if (length)
                *length = gdk_pixbuf_get_byte_length (pixbuf);

	return gdk_pixbuf_get_pixels(pixbuf);
}

static void
erpi_clipboard_image_received(GtkClipboard *clipboard, GdkPixbuf *pixbuf, gpointer user_data)
{
	GtkIMHtml *imhtml = GTK_IMHTML(user_data);
	if (pixbuf == NULL) {
		// No images in the clipboard
		return;
	}
	
	guint len;
	guchar *data = gdk_pixbuf_get_pixels_with_length(pixbuf, &len);
	int img_id = purple_imgstore_add_with_id(data, len, "clipboard_image");

	GtkTextIter iter;
	GtkTextMark *ins;
	ins = gtk_text_buffer_get_insert(gtk_text_view_get_buffer(GTK_TEXT_VIEW(imhtml)));
	gtk_text_buffer_get_iter_at_mark(gtk_text_view_get_buffer(GTK_TEXT_VIEW(imhtml)),
									 &iter, ins);
	gtk_imhtml_insert_image_at_iter(imhtml, img_id, &iter);
	purple_imgstore_unref_by_id(img_id);
}


static void
erpi_paste_clipboard_cb(GtkIMHtml *imhtml, const char *str)
{
	if (!gtk_text_view_get_editable(GTK_TEXT_VIEW(imhtml)))
		return;
	
	purple_debug_info("paste_image", "Pasting clipboard image %s\n", str);
	GtkClipboard *clipboard = gtk_widget_get_clipboard(GTK_WIDGET(imhtml), GDK_SELECTION_CLIPBOARD);
	gtk_clipboard_request_image(clipboard, erpi_clipboard_image_received, imhtml);	
}


static void
detach_from_gtkconv(PidginConversation *gtkconv, gpointer null)
{
	g_signal_handlers_disconnect_by_func(G_OBJECT(gtkconv->imhtml), G_CALLBACK(erpi_paste_clipboard_cb), NULL);
}

static void
detach_from_pidgin_window(PidginWindow *win, gpointer null)
{
	g_list_foreach(pidgin_conv_window_get_gtkconvs(win), (GFunc)detach_from_gtkconv, NULL);
}

static void
attach_to_gtkconv(PidginConversation *gtkconv, gpointer null)
{
	detach_from_gtkconv(gtkconv, NULL);
	g_signal_connect(G_OBJECT(gtkconv->imhtml), "paste", G_CALLBACK(erpi_paste_clipboard_cb), NULL);
}

static void
attach_to_pidgin_window(PidginWindow *win, gpointer null)
{
	g_list_foreach(pidgin_conv_window_get_gtkconvs(win), (GFunc)attach_to_gtkconv, NULL);
}

static void
detach_from_all_windows(void)
{
	g_list_foreach(pidgin_conv_windows_get_list(), (GFunc)detach_from_pidgin_window, NULL);
}

static void
attach_to_all_windows(void)
{
	g_list_foreach(pidgin_conv_windows_get_list(), (GFunc)attach_to_pidgin_window, NULL);
}

static void
conv_created(PidginConversation *gtkconv, gpointer null)
{
	PidginWindow *win;

	win = pidgin_conv_get_window(gtkconv);
	if (!win)
		return;

	detach_from_pidgin_window(win, NULL);
	attach_to_pidgin_window(win, NULL);
}

static gboolean
plugin_load(PurplePlugin *plugin)
{
	attach_to_all_windows();

	purple_signal_connect(pidgin_conversations_get_handle(), "conversation-displayed",
						plugin, PURPLE_CALLBACK(conv_created), NULL);

	return TRUE;
}

static gboolean
plugin_unload(PurplePlugin *plugin)
{
	detach_from_all_windows();

	return TRUE;
}

static PurplePluginInfo info =
{
	PURPLE_PLUGIN_MAGIC,
	2,  // PURPLE_MAJOR_VERSION,
	10, // PURPLE_MINOR_VERSION,
	PURPLE_PLUGIN_STANDARD,                           /**< type           */
	PIDGIN_PLUGIN_TYPE,                               /**< ui_requirement */
	0,                                                /**< flags          */
	NULL,                                             /**< dependencies   */
	PURPLE_PRIORITY_DEFAULT,                          /**< priority       */

	PLUGIN_ID,                                        /**< id             */
	"Paste Images",                                   /**< name           */
	"0.1",                                            /**< version        */
	                                                  /**  summary        */
	"Allow pasting images into a coversation window",
	                                                  /**  description    */
	"Allow pasting images into a coversation window",
	"Eion Robb <eionrobb@gmail.com>",
	"https://github.com/EionRobb",                    /**< homepage       */

	plugin_load,                                      /**< load           */
	plugin_unload,                                    /**< unload         */
	NULL,                                             /**< destroy        */

	NULL,                                             /**< ui_info        */
	NULL,                                             /**< extra_info     */
	NULL,                                             /**< prefs_info     */
	NULL,                                             /**< actions        */

	/* padding */
	NULL,
	NULL,
	NULL,
	NULL
};

static void
init_plugin(PurplePlugin *plugin)
{
}

PURPLE_INIT_PLUGIN(paste_image, init_plugin, info)
