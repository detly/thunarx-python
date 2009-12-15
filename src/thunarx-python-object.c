/*-
 * Copyright (c) 2006 Peter de Ridder <peter@xfce.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>

#include "thunarx-python-object.h"
#include "thunarx-python.h"

#include <thunarx/thunarx.h>

#include <pygobject.h>

#include <string.h>

#define METHOD_PREFIX ""

static GObjectClass *parent_class;

/* These macros assumes the following things:
 *   a METHOD_NAME is defined with is a string
 *   a goto label called beach
 *   the return value is called ret
 */

#define CHECK_METHOD_NAME(self)                                         \
	if (!PyObject_HasAttrString(self, METHOD_NAME))                       \
		goto beach;
	
#define CONVERT_LIST(py_files, files)                                   \
	{                                                                     \
		GList *l;                                                           \
    py_files = PyList_New(0);                                           \
		for (l = files; l; l = l->next) {                                   \
      PyObject *obj = pygobject_new((GObject*)l->data);                 \
			PyList_Append(py_files, obj);							                        \
			Py_DECREF(obj);                                                   \
		}                                                                   \
	}

#define HANDLE_RETVAL(py_ret)                                           \
  if (!py_ret) {                                                        \
		PyErr_Print();                                                      \
		goto beach;                                                         \
 	}                                                                     \
 	else if (py_ret == Py_None) {                                         \
 		goto beach;                                                         \
	}

#define HANDLE_LIST(py_ret, type, type_name)                            \
    {                                                                   \
      Py_ssize_t i = 0;                                                 \
    	if (!PySequence_Check(py_ret) || PyString_Check(py_ret)) {        \
    		PyErr_SetString(PyExc_TypeError,                                \
    						METHOD_NAME " must return a sequence");                 \
    		goto beach;                                                     \
    	}                                                                 \
    	for (i = 0; i < PySequence_Size (py_ret); i++) {                  \
    		PyGObject *py_item;                                             \
    		py_item = (PyGObject*)PySequence_GetItem (py_ret, i);           \
    		if (!pygobject_check(py_item, &Py##type##_Type)) {              \
    			PyErr_SetString(PyExc_TypeError,                              \
    							METHOD_NAME                                           \
    							" must return a sequence of "                         \
    							type_name);                                           \
    			goto beach;                                                   \
    		}                                                               \
    		ret = g_list_append (ret, (type*) g_object_ref(py_item->obj));  \
            Py_DECREF(py_item);                                         \
    	}                                                                 \
    }


static void
thunarx_python_object_menu_provider_iface_init (ThunarxMenuProviderIface *iface)
{
	iface->get_file_actions = thunarx_python_object_get_file_actions;
	iface->get_folder_actions = thunarx_python_object_get_folder_actions;
	iface->get_dnd_actions = thunarx_python_object_get_dnd_actions;
}



#define METHOD_NAME "get_file_actions"
static GList *
thunarx_python_object_get_file_actions (ThunarxMenuProvider *provider,
                                        GtkWidget *window,
                                        GList *files)
{
  ThunarxPythonObject *object = (ThunarxPythonObject*)provider;
  GList *ret = NULL;
  PyObject *py_ret = NULL, *py_files;
	PyGILState_STATE state = pyg_gil_state_ensure();
	
  debug_enter();

	CHECK_METHOD_NAME(object->instance);

	CONVERT_LIST(py_files, files);

  py_ret = PyObject_CallMethod(object->instance, METHOD_PREFIX METHOD_NAME,
								 "(NN)", pygobject_new((GObject *)window), py_files);

	HANDLE_RETVAL(py_ret);

	HANDLE_LIST(py_ret, GtkAction, "gtk.Action");

beach:
  Py_XDECREF(py_ret);
  pyg_gil_state_release(state);
  return ret;
}
#undef METHOD_NAME



#define METHOD_NAME "get_folder_actions"
static GList *
thunarx_python_object_get_folder_actions (ThunarxMenuProvider *provider,
 		  									                  GtkWidget *window,
	  										                  ThunarxFileInfo *folder)
{
	ThunarxPythonObject *object = (ThunarxPythonObject*)provider;
  GList *ret = NULL;
  PyObject *py_ret = NULL;
	PyGILState_STATE state = pyg_gil_state_ensure();
	
  debug_enter();

	CHECK_METHOD_NAME(object->instance);

  py_ret = PyObject_CallMethod(object->instance, METHOD_PREFIX METHOD_NAME,
								 "(NN)",
								 pygobject_new((GObject *)window),
								 pygobject_new((GObject *)folder));
								 
	HANDLE_RETVAL(py_ret);

	HANDLE_LIST(py_ret, GtkAction, "gtk.Action");
	
beach:
  Py_XDECREF(py_ret);
  pyg_gil_state_release(state);
  return ret;
}
#undef METHOD_NAME



#define METHOD_NAME "get_dnd_actions"
static GList *
thunarx_python_object_get_dnd_actions (ThunarxMenuProvider *provider,
                                        GtkWidget *window,
                                        ThunarxFileInfo *folder,
                                        GList *files)
{
  ThunarxPythonObject *object = (ThunarxPythonObject*)provider;
  GList *ret = NULL;
  PyObject *py_ret = NULL, *py_files;
	PyGILState_STATE state = pyg_gil_state_ensure();
	
  debug_enter();

	CHECK_METHOD_NAME(object->instance);

	CONVERT_LIST(py_files, files);

  py_ret = PyObject_CallMethod(object->instance, METHOD_PREFIX METHOD_NAME,
								 "(NN)", 
								 pygobject_new((GObject *)window), 
								 pygobject_new((GObject *)folder),
								 py_files);

	HANDLE_RETVAL(py_ret);

	HANDLE_LIST(py_ret, GtkAction, "gtk.Action");

beach:
  Py_XDECREF(py_ret);
  pyg_gil_state_release(state);
  return ret;
}
#undef METHOD_NAME



/*
static void
thunarx_python_object_property_page_provider_iface_init (ThunarxPropertyPageProviderIface *iface)
{
	iface->get_pages = thunarx_python_object_get_property_pages;
}



static void
thunarx_python_object_file_info_iface_init (ThunarxFileInfoIface *iface)
{
	iface->file_changed = thunarx_python_object_file_changed;
	iface->file_renamed = thunarx_python_object_file_renamed;
}



static void
thunarx_python_object_renamer_provider_iface_init (ThunarxRenamerProviderIface *iface)
{
	iface->get_renamers = thunarx_python_object_get_renamers;
}



static void
thunarx_python_object_preferences_provider_iface_init (ThunarxPreferencesProviderIface *iface)
{
	iface->get_actions = thunarx_python_object_get_actions;
}
*/


static void 
thunarx_python_object_instance_init (ThunarxPythonObject *object)
{
	ThunarxPythonObjectClass *class;
  debug_enter();

	class = (ThunarxPythonObjectClass*)(((GTypeInstance*)object)->g_class);

	object->instance = PyObject_CallObject(class->type, NULL);
	if (object->instance == NULL)
		PyErr_Print();
}

static void
thunarx_python_object_finalize (GObject *object)
{
  debug_enter();

	Py_DECREF(((ThunarxPythonObject *)object)->instance);
}

static void
thunarx_python_object_class_init (ThunarxPythonObjectClass *klass,
								                  gpointer class_data)
{
	debug_enter();

	parent_class = g_type_class_peek_parent (klass);
	
	klass->type = (PyObject*)class_data;
	
	G_OBJECT_CLASS (klass)->finalize = thunarx_python_object_finalize;
}



GType 
thunarx_python_object_get_type (ThunarxProviderPlugin *plugin, PyObject *type)
{
	GTypeInfo *info;
	const char *type_name;
	GType gtype;
/*
	static const GInterfaceInfo file_info_iface_info = {
		(GInterfaceInitFunc) thunarx_python_object_file_info_iface_init,
		NULL,
		NULL
	};
	  
	static const GInterfaceInfo property_page_provider_iface_info = {
		(GInterfaceInitFunc) thunarx_python_object_property_page_provider_iface_init,
		NULL,
		NULL
	};
*/
	static const GInterfaceInfo menu_provider_iface_info = {
		(GInterfaceInitFunc) thunarx_python_object_menu_provider_iface_init,
		NULL,
		NULL
	};
/*
	static const GInterfaceInfo renamer_provider_iface_info = {
		(GInterfaceInitFunc) thunarx_python_object_renamer_provider_iface_init,
		NULL,
		NULL
	};

	static const GInterfaceInfo preferences_provider_iface_info = {
		(GInterfaceInitFunc) thunarx_python_object_preferences_provider_iface_init,
		NULL,
		NULL
	};
*/
	debug_enter_args("type=%s", PyString_AsString(PyObject_GetAttrString(type, "__name__")));
	info = g_new0 (GTypeInfo, 1);
	
	info->class_size = sizeof (ThunarxPythonObjectClass);
	info->class_init = (GClassInitFunc)thunarx_python_object_class_init;
	info->instance_size = sizeof (ThunarxPythonObject);
	info->instance_init = (GInstanceInitFunc)thunarx_python_object_instance_init;

	info->class_data = type;
	Py_INCREF(type);

	type_name = g_strdup_printf("%s+ThunarxPython",
								PyString_AsString(PyObject_GetAttrString(type, "__name__")));

	gtype = thunarx_provider_plugin_register_type (plugin, 
										 G_TYPE_OBJECT,
										 type_name,
										 info, 0);
/*
	if (PyObject_IsSubclass(type, (PyObject*)&PyThunarxFileInfo_Type)) {
		thunarx_provider_plugin_add_interface (plugin, gtype, 
									 THUNARX_TYPE_FILE_INFO,
									 &file_info_iface_info);
	}

	if (PyObject_IsSubclass(type, (PyObject*)&PyThunarxPropertyPageProvider_Type)) {
		thunarx_provider_plugin_add_interface (plugin, gtype, 
									 THUNARX_TYPE_PROPERTY_PAGE_PROVIDER,
									 &property_page_provider_iface_info);
	}
*/
	if (PyObject_IsSubclass(type, (PyObject*)&PyThunarxMenuProvider_Type)) {
		thunarx_provider_plugin_add_interface (plugin, gtype, 
									 THUNARX_TYPE_MENU_PROVIDER,
									 &menu_provider_iface_info);
	}
/*
	if (PyObject_IsSubclass(type, (PyObject*)&PyThunarxRenamerProvider_Type)) {
		thunarx_provider_plugin_add_interface (plugin, gtype, 
									 THUNARX_TYPE_RENAMER_PROVIDER,
									 &renamer_provider_iface_info);
	}
	
	if (PyObject_IsSubclass(type, (PyObject*)&PyThunarxPreferencesProvider_Type)) {
		thunarx_provider_plugin_add_interface (plugin, gtype, 
									 THUNARX_TYPE_PREFERENCES_PROVIDER,
									 &preferences_provider_iface_info);
	}
*/	
	return gtype;
}
