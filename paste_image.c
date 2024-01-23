
#include "purple.h"
#include "gtkplugin.h"
#include "gtkconv.h"
#include "gtkimhtml.h"
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#define PLUGIN_ID "eionrobb_paste_image"

static void
erpi_clipboard_image_received(GtkClipboard *clipboard, GdkPixbuf *pixbuf, gpointer user_data)
{
	GtkIMHtml *imhtml = GTK_IMHTML(user_data);
	if (pixbuf == NULL) {
		// No images in the clipboard
		return;
	}
	
	gsize len;
	gchar *data;
	gdk_pixbuf_save_to_buffer(pixbuf, &data, &len, "png", NULL, NULL);
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
	if (!(imhtml->format_functions & GTK_IMHTML_IMAGE))
		return;
	
	purple_debug_info("paste_image", "Pasting clipboard image\n");
	GtkClipboard *clipboard = gtk_widget_get_clipboard(GTK_WIDGET(imhtml), GDK_SELECTION_CLIPBOARD);
	gtk_clipboard_request_image(clipboard, erpi_clipboard_image_received, imhtml);	
}


static void
detach_from_gtkconv(PidginConversation *gtkconv, gpointer null)
{
	g_signal_handlers_disconnect_by_func(G_OBJECT(gtkconv->entry), G_CALLBACK(erpi_paste_clipboard_cb), NULL);
}

static void
detach_from_pidgin_window(PidginWindow *win, gpointer null)
{
	g_list_foreach(pidgin_conv_window_get_gtkconvs(win), (GFunc)detach_from_gtkconv, NULL);
}


static void 
erpi_hijack_menu_cb(GtkIMHtml *imhtml, GtkMenu *menu, gpointer data)
{
	GList *children, *iter;
	GtkStockItem stock_item;
	GtkClipboard *clipboard = gtk_widget_get_clipboard(GTK_WIDGET(imhtml), GDK_SELECTION_CLIPBOARD);
	gboolean image_in_buffer = gtk_clipboard_wait_is_image_available(clipboard);

	if (!image_in_buffer)
		return;
		
	if (!(imhtml->format_functions & GTK_IMHTML_IMAGE))
		return;

	gtk_stock_lookup(GTK_STOCK_PASTE, &stock_item);

	children = gtk_container_get_children(GTK_CONTAINER(menu));
	for (iter = children; iter != NULL; iter = iter->next) {
		GtkMenuItem *item = GTK_MENU_ITEM(iter->data);
		GtkWidget *child = GTK_BIN(item)->child;
		// Look for GTK_STOCK_PASTE and activate it if there's image data to paste
		if (child && GTK_IS_LABEL(child) && purple_strequal(gtk_label_get_label(GTK_LABEL(child)), stock_item.label)) {
			
			gtk_widget_set_sensitive(GTK_WIDGET(item), image_in_buffer);
			break;
		}
	}
}

static void
attach_to_gtkconv(PidginConversation *gtkconv, gpointer null)
{
	detach_from_gtkconv(gtkconv, NULL);
	g_signal_connect(G_OBJECT(gtkconv->entry), "paste", G_CALLBACK(erpi_paste_clipboard_cb), NULL);
	g_signal_connect(G_OBJECT(gtkconv->entry), "paste-clipboard", G_CALLBACK(erpi_paste_clipboard_cb), NULL);
	g_signal_connect(G_OBJECT(gtkconv->entry), "populate-popup", G_CALLBACK(erpi_hijack_menu_cb), NULL);
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
