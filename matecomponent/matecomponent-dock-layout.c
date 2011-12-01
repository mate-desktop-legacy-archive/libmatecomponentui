/* matecomponent-dock-layout.c

   Copyright (C) 1998 Free Software Foundation

   All rights reserved.

   The Mate Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Mate Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Mate Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Ettore Perazzoli <ettore@comm2000.it>
*/
/*
  @NOTATION@
*/

#include <config.h>
#include <string.h>
#include <gtk/gtk.h>
#include <stdio.h>

#include "matecomponent-dock-layout.h"

/* TODO: handle incorrect MATECOMPONENT_DOCK_ITEM_BEH_EXCLUSIVE situations.  */

struct _MateComponentDockLayoutPrivate
{
	int dummy;
	/* Nothing right now, needs to get filled with the private things */
	/* XXX: When stuff is added, uncomment the allocation in the
	 * matecomponent_dock_layout_init function! */
};

static GObjectClass *parent_class = NULL;



static void   matecomponent_dock_layout_class_init   (MateComponentDockLayoutClass  *class);

static void   matecomponent_dock_layout_instance_init(MateComponentDockLayout *layout);

static void   matecomponent_dock_layout_finalize     (GObject *object);

static gint   item_compare_func              (gconstpointer a,
                                              gconstpointer b);

static gint   compare_item_by_name           (gconstpointer a,
                                              gconstpointer b);

static gint   compare_item_by_pointer        (gconstpointer a,
                                              gconstpointer b);

static GList *find                           (MateComponentDockLayout *layout,
                                              gconstpointer a,
                                              GCompareFunc func);

static void   remove_item                    (MateComponentDockLayout *layout,
                                              GList *list);


static void
matecomponent_dock_layout_class_init (MateComponentDockLayoutClass  *class)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);

  gobject_class->finalize = matecomponent_dock_layout_finalize;

  parent_class = g_type_class_ref (G_TYPE_OBJECT);
}

static void
matecomponent_dock_layout_instance_init (MateComponentDockLayout *layout)
{
  layout->_priv = NULL;
  /* XXX: when there is some private stuff enable this
  layout->_priv = g_new0(MateComponentDockLayoutPrivate, 1);
  */
  layout->items = NULL;
}

static void
matecomponent_dock_layout_finalize (GObject *object)
{
  MateComponentDockLayout *layout;

  layout = MATECOMPONENT_DOCK_LAYOUT (object);

  while (layout->items)
    remove_item (layout, layout->items);

  /* Free the private structure */
  g_free (layout->_priv);
  layout->_priv = NULL;

  if (G_OBJECT_CLASS (parent_class)->finalize)
    (* G_OBJECT_CLASS (parent_class)->finalize) (object);
}



static gint
item_compare_func (gconstpointer a,
                   gconstpointer b)
{
  const MateComponentDockLayoutItem *item_a, *item_b;

  item_a = a;
  item_b = b;

  if (item_a->placement != item_b->placement)
    return item_b->placement - item_a->placement;

  if (item_a->placement == MATECOMPONENT_DOCK_FLOATING)
    return 0; /* Floating items don't need to be ordered.  */
  else
    {
      if (item_a->position.docked.band_num != item_b->position.docked.band_num)
        return (item_b->position.docked.band_num
                - item_a->position.docked.band_num);

      return (item_b->position.docked.band_position
              - item_a->position.docked.band_position);
    }
}

static gint
compare_item_by_name (gconstpointer a, gconstpointer b)
{
  const MateComponentDockItem *item;
  const gchar *name;

  item = b;
  name = a;

  return strcmp (name, item->name);
}

static gint
compare_item_by_pointer (gconstpointer a, gconstpointer b)
{
  return a != b;
}

static GList *
find (MateComponentDockLayout *layout, gconstpointer data, GCompareFunc func)
{
  GList *p;

  for (p = layout->items; p != NULL; p = p->next)
    {
      MateComponentDockLayoutItem *item;

      item = p->data;
      if (! (* func) (data, item->item))
        return p;
    }

  return NULL;
}

static void
remove_item (MateComponentDockLayout *layout,
             GList *list)
{
  MateComponentDockItem *item;

  item = ((MateComponentDockLayoutItem *) list->data)->item;

  g_object_unref (GTK_WIDGET (item));

  layout->items = g_list_remove_link (layout->items, list);

  g_free (list->data);
  g_list_free (list);
}



GType
matecomponent_dock_layout_get_type (void)
{
  static GType layout_type = 0;
	
  if (layout_type == 0)
    {
      GTypeInfo layout_info = {
	sizeof (MateComponentDockLayoutClass),
	NULL, NULL,
	(GClassInitFunc)matecomponent_dock_layout_class_init,
	NULL, NULL,
	sizeof (MateComponentDockLayout),
	0,
	(GInstanceInitFunc)matecomponent_dock_layout_instance_init
      };

      layout_type = g_type_register_static (G_TYPE_OBJECT, "MateComponentDockLayout", &layout_info, 0);
    }
  
  return layout_type;
}

/**
 * matecomponent_dock_layout_new:
 * 
 * Description: Create a new #MateComponentDockLayout widget.
 * 
 * Returns: The new #MateComponentDockLayout widget.
 **/
   
MateComponentDockLayout *
matecomponent_dock_layout_new (void)
{
  return MATECOMPONENT_DOCK_LAYOUT (g_object_new (MATECOMPONENT_TYPE_DOCK_LAYOUT, NULL));
}

/**
 * matecomponent_dock_layout_add_item:
 * @layout: A #MateComponentDockLayout widget
 * @item: The dock item to be added to @layout
 * @placement: Placement of @item in @layout
 * @band_num: Band number
 * @band_position: Position within the band
 * @offset: Distance from the previous element in the band
 * 
 * Description: Add @item to @layout with the specified parameters.
 * 
 * Returns: %TRUE if the operation succeeds, %FALSE if it fails.
 **/
gboolean
matecomponent_dock_layout_add_item (MateComponentDockLayout *layout,
                            MateComponentDockItem *item,
                            MateComponentDockPlacement placement,
                            gint band_num,
                            gint band_position,
                            gint offset)
{
  MateComponentDockLayoutItem *new;

  new = g_new (MateComponentDockLayoutItem, 1);
  new->item = item;
  new->placement = placement;
  new->position.docked.band_num = band_num;
  new->position.docked.band_position = band_position;
  new->position.docked.offset = offset;

  layout->items = g_list_prepend (layout->items, new);

  g_object_ref (item);

  return TRUE;
}

/**
 * matecomponent_dock_layout_add_floating_item:
 * @layout: A #MateComponentDockLayout widget
 * @item: The dock item to be added to @layout
 * @x: X-coordinate for the floating item
 * @y: Y-coordinate for the floating item
 * @orientation: Orientation for the floating item
 * 
 * Description: Add @item to @layout as a floating item with the
 * specified (@x, @y) position and @orientation.
 * 
 * Returns: %TRUE if the operation succeeds, %FALSE if it fails.
 **/
   
gboolean
matecomponent_dock_layout_add_floating_item (MateComponentDockLayout *layout,
                                     MateComponentDockItem *item,
                                     gint x, gint y,
                                     GtkOrientation orientation)
{
  MateComponentDockLayoutItem *new;

  new = g_new (MateComponentDockLayoutItem, 1);
  new->item = item;
  new->placement = MATECOMPONENT_DOCK_FLOATING;
  new->position.floating.x = x;
  new->position.floating.y = y;
  new->position.floating.orientation = orientation;

  layout->items = g_list_prepend (layout->items, new);

  g_object_ref (item);

  return TRUE;
}

/**
 * matecomponent_dock_layout_get_item:
 * @layout: A #MateComponentDockLayout widget
 * @item: The #MateComponentDockItem to be retrieved
 * 
 * Description: Retrieve a layout item.
 * 
 * Returns: The retrieved #MateComponentDockLayoutItem widget.
 **/
MateComponentDockLayoutItem *
matecomponent_dock_layout_get_item (MateComponentDockLayout *layout,
                            MateComponentDockItem *item)
{
  GList *list;

  list = find (layout, item, compare_item_by_pointer);

  if (list == NULL)
    return NULL;
  else
    return list->data;
}

/**
 * matecomponent_dock_layout_get_item_by_name:
 * @layout: A #MateComponentDockLayout widget
 * @name: Name of the item to be retrieved
 * 
 * Description: Retrieve the dock item named @name.
 * 
 * Returns: The named #MateComponentDockLayoutItem widget.
 **/
MateComponentDockLayoutItem *
matecomponent_dock_layout_get_item_by_name (MateComponentDockLayout *layout,
                                    const gchar *name)
{
  GList *list;

  list = find (layout, name, compare_item_by_name);

  if (list == NULL)
    return NULL;
  else
    return list->data;
}

/**
 * matecomponent_dock_layout_remove_item:
 * @layout: A #MateComponentDockLayout widget
 * @item: The #MateComponentDockItem to be removed
 * 
 * Description: Remove the specified @item from @layout.
 * 
 * Returns: %TRUE if the operation succeeds, %FALSE if it fails.
 **/
gboolean
matecomponent_dock_layout_remove_item (MateComponentDockLayout *layout,
                               MateComponentDockItem *item)
{
  GList *list;

  list = find (layout, item, compare_item_by_pointer);
  if (list == NULL)
    return FALSE;

  remove_item (layout, list);

  return TRUE;
}

/**
 * matecomponent_dock_layout_remove_item_by_name:
 * @layout: A #MateComponentDockLayout widget
 * @name: Name of the #MateComponentDockItem to be removed
 * 
 * Description: Remove the item named @name from @layout.
 * 
 * Returns: %TRUE if the operation succeeds, %FALSE if it fails.
 **/
gboolean
matecomponent_dock_layout_remove_item_by_name (MateComponentDockLayout *layout,
                                       const gchar *name)
{
  GList *list;

  list = find (layout, name, compare_item_by_name);
  if (list == NULL)
    return FALSE;

  remove_item (layout, list);

  return TRUE;
}



/**
 * matecomponent_dock_layout_add_to_dock:
 * @layout: A #MateComponentDockLayout widget
 * @dock: The #MateComponentDock widget the layout items must be added to
 * 
 * Description: Add all the items in @layout to the specified @dock.
 * 
 * Returns: %TRUE if the operation succeeds, %FALSE if it fails.
 **/
gboolean
matecomponent_dock_layout_add_to_dock (MateComponentDockLayout *layout,
                               MateComponentDock *dock)
{
  MateComponentDockLayoutItem *item;
  GList *lp;
  MateComponentDockPlacement last_placement;
  gint last_band_num;

  if (layout->items == NULL)
    return FALSE;

  layout->items = g_list_sort (layout->items, item_compare_func);

  item = layout->items->data;

  last_placement = MATECOMPONENT_DOCK_FLOATING;
  last_band_num = 0;

  for (lp = layout->items; lp != NULL; lp = lp->next)
    {
      item = lp->data;

      if (item->placement == MATECOMPONENT_DOCK_FLOATING)
        {
          matecomponent_dock_add_floating_item (dock,
                                        item->item,
                                        item->position.floating.x,
                                        item->position.floating.y,
                                        item->position.floating.orientation);
        }
      else
        {
          gboolean need_new;

          if (last_placement != item->placement
              || last_band_num != item->position.docked.band_num)
            need_new = TRUE;
          else
            need_new = FALSE;

          matecomponent_dock_add_item (dock,
                               item->item,
                               item->placement,
                               0,
                               0,
                               item->position.docked.offset,
                               need_new);

          last_band_num = item->position.docked.band_num;
          last_placement = item->placement;
        }

      gtk_widget_show (GTK_WIDGET (item->item));
    }

  return TRUE;
}



/* Layout string functions.  */

/**
 * matecomponent_dock_layout_create_string:
 * @layout: A #MateComponentDockLayout widget
 * 
 * Description: Generate a string describing the layout in @layout.
 * 
 * Returns: The (malloced) layout string for @layout.
 **/
gchar *
matecomponent_dock_layout_create_string (MateComponentDockLayout *layout)
{
  GList *lp;
  guint tmp_count, tmp_alloc;
  gchar **tmp;
  gchar *retval;

  if (layout->items == NULL)
    return NULL;

  tmp_alloc = 512;
  tmp = g_new (gchar *, tmp_alloc);

  tmp_count = 0;

  for (lp = layout->items; lp != NULL; lp = lp->next)
    {
      MateComponentDockLayoutItem *i;

      i = lp->data;

      if (tmp_alloc - tmp_count <= 2)
        {
          tmp_alloc *= 2;
          tmp = g_renew (char *, tmp, tmp_alloc);
        }

      if (i->placement == MATECOMPONENT_DOCK_FLOATING)
        tmp[tmp_count] = g_strdup_printf ("%s\\%d,%d,%d,%d",
                                          i->item->name ? i->item->name : "",
                                          (gint) i->placement,
                                          i->position.floating.x,
                                          i->position.floating.y,
                                          i->position.floating.orientation);
      else
        tmp[tmp_count] = g_strdup_printf ("%s\\%d,%d,%d,%d",
                                          i->item->name ? i->item->name : "",
                                          (gint) i->placement,
                                          i->position.docked.band_num,
                                          i->position.docked.band_position,
                                          i->position.docked.offset);

      tmp_count++;
    }

  tmp[tmp_count] = NULL;

  retval = g_strjoinv ("\\", tmp);
  g_strfreev (tmp);

  return retval;
}

/**
 * matecomponent_dock_layout_parse_string:
 * @layout: A #MateComponentDockLayout widget
 * @string: A layout string to be parsed
 * 
 * Description: Parse the layout string @string, and move around the
 * items in @layout accordingly.
 * 
 * Returns: %TRUE if the operation succeeds, %FALSE if it fails.
 **/
gboolean
matecomponent_dock_layout_parse_string (MateComponentDockLayout *layout,
				const gchar *string)
{
  gchar **tmp, **p;

  if (string == NULL)
    return FALSE;

  tmp = g_strsplit (string, "\\", 0);
  if (tmp == NULL)
    return FALSE;

  p = tmp;
  while (*p != NULL)
    {
      GList *lp;

      if (*(p + 1) == NULL)
        {
          g_strfreev (tmp);
          return FALSE;
        }

      lp = find (layout, *p, compare_item_by_name);

      if (lp != NULL)
        {
          MateComponentDockLayoutItem *i;
          gint p1, p2, p3, p4;

          if (sscanf (*(p + 1), "%d,%d,%d,%d", &p1, &p2, &p3, &p4) != 4)
            {
              g_strfreev (tmp);
              return FALSE;
            }

          if (p1 != (gint) MATECOMPONENT_DOCK_TOP
              && p1 != (gint) MATECOMPONENT_DOCK_BOTTOM
              && p1 != (gint) MATECOMPONENT_DOCK_LEFT
              && p1 != (gint) MATECOMPONENT_DOCK_RIGHT
              && p1 != (gint) MATECOMPONENT_DOCK_FLOATING)
            return FALSE;
          
          i = lp->data;

          i->placement = (MateComponentDockPlacement) p1;

          if (i->placement == MATECOMPONENT_DOCK_FLOATING)
            {
              i->position.floating.x = p2;
              i->position.floating.y = p3;
              i->position.floating.orientation = p4;
            }
          else
            {
              i->position.docked.band_num = p2;
              i->position.docked.band_position = p3;
              i->position.docked.offset = p4;
            }
        }

      p += 2;
    }

  g_strfreev (tmp);

  return TRUE;
}
