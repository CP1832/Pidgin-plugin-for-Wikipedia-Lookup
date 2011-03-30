/*
 *  Wikipedia Lookup - A third-party Pidgin plug-in which offers 
 *					  you the possibility to look up received and
 *					  typed words on Wikipedia.
 *
 *  Copyright (C) 2011 Hendrik Kunert kunerd@users.sourceforge.net
 *
 *  This file is part of wplookup.
 *
 *  wplookup is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Foobar is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with wplookup.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

#include "wikiinfo.h"

void wpl_set_url(guchar *name, guchar *url)
{
	int size = 0;

	/*name */
	/* add 1 for \0 and the other for / in the url */
	size = strlen((gchar*)name)+1;

	if(wpl_settings.language != NULL)
		g_free(wpl_settings.language);

	wpl_settings.language = (guchar *) malloc(size*sizeof(guchar));
	
	if(wpl_settings.language != NULL)
	{
		g_sprintf((gchar*)wpl_settings.language,"%s", (gchar*)name);
	}

	/* url */
	size = strlen((gchar*)url)+strlen(WIKIPEDIA_PATH)+1;
	if(wpl_settings.wikipedia_search_url != NULL)
		g_free(wpl_settings.wikipedia_search_url);

	wpl_settings.wikipedia_search_url = (guchar *) malloc(size*sizeof(guchar));
	
	if(wpl_settings.wikipedia_search_url != NULL)
	{
		g_sprintf((gchar*)wpl_settings.wikipedia_search_url,"%s%s", (gchar*)url, WIKIPEDIA_PATH);
	}
}

size_t
WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)data;

	mem->memory = realloc(mem->memory, mem->size + realsize + 1);
	if (mem->memory == NULL) {
		/* out of memory! */ 
		printf("not enough memory (realloc returned NULL)\n");
		exit(EXIT_FAILURE);
	}

	memcpy(&(mem->memory[mem->size]), ptr, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

xmlXPathObjectPtr
getnodeset (xmlDocPtr doc, xmlChar *xpath){

	xmlXPathContextPtr context;
	xmlXPathObjectPtr result;

	context = xmlXPathNewContext(doc);
	if (context == NULL) {
		return NULL;
	}
	result = xmlXPathEvalExpression(xpath, context);
	xmlXPathFreeContext(context);
	if (result == NULL) {
		return NULL;
	}
	if(xmlXPathNodeSetIsEmpty(result->nodesetval)){
		xmlXPathFreeObject(result);
		return NULL;
	}
	return result;
}

GtkTreeModel *getWikipediaLanguages(GtkTreeIter *sel_iter)
{
	int i = 0;
	CURL *curl_handle=NULL;
	xmlDoc *doc = NULL;
	xmlDoc *subDoc = NULL;
	xmlNode *root_element = NULL;
	xmlXPathObjectPtr resultName = NULL;
	xmlXPathObjectPtr resultUrl = NULL;
	xmlChar *xpathName = (xmlChar*) "/api/sitematrix/language";
	xmlChar *xpathUrl = (xmlChar*) "/language/site/site[@code=\"wiki\"]/@url";
	xmlNodeSetPtr nodesetName = NULL;
	xmlNodeSetPtr nodesetUrl= NULL;
	xmlChar *url = NULL;
	xmlChar *name = NULL;
	GtkListStore  *store = NULL;
	GtkTreeIter    iter;
	
	struct MemoryStruct chunk;

	chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */ 
	chunk.size = 0;    /* no data at this point */

	store = gtk_list_store_new (NUM_COLS, G_TYPE_STRING, G_TYPE_STRING);

	curl_global_init(CURL_GLOBAL_NOTHING);

	/* init the curl session */ 
	curl_handle = curl_easy_init();

	/* specify URL to get */ 
	curl_easy_setopt(curl_handle, CURLOPT_URL, WPL_WIKIPEDIA_SITEMATRIX);

	/* send all data to this function  */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

	/* we pass our 'chunk' struct to the callback function */ 
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

	/* some servers don't like requests that are made without a user-agent field, so we provide one */ 
	curl_easy_setopt (curl_handle, CURLOPT_USERAGENT, WPL_USER_AGENT);

	/* get it! */ 
	curl_easy_perform(curl_handle);

	/* cleanup curl stuff */ 
	curl_easy_cleanup(curl_handle);

	/* we're done with libcurl, so clean it up */
	curl_global_cleanup();

	doc = xmlParseMemory(chunk.memory, chunk.size);
	if(doc == NULL){
		printf("no valid xml");
	}

	resultName = getnodeset (doc, xpathName);
	                 
	if (resultName != NULL) {
		nodesetName = resultName->nodesetval;

		for (i=0; i < nodesetName->nodeNr; i++) {
			
			subDoc = xmlNewDoc(BAD_CAST "1.0");
			if(subDoc == NULL){
				printf("no valid xml");
			}
			
			root_element = nodesetName->nodeTab[i];
			xmlDocSetRootElement(subDoc, root_element);
			resultUrl = getnodeset (subDoc, xpathUrl);
			if (resultUrl != NULL) {
				nodesetUrl = resultUrl->nodesetval;
				if(nodesetUrl->nodeNr)
				{
					url = xmlNodeListGetString(subDoc, nodesetUrl->nodeTab[0]->xmlChildrenNode, 1);
					name = xmlGetProp(nodesetName->nodeTab[i], (const xmlChar*)"name");
					
					  gtk_list_store_append (store, &iter);
					  gtk_list_store_set (store, &iter,
										  COL_NAME, name,
										  COL_URL, url,
										  -1);

					if(strcmp((gchar*)name,(gchar*)wpl_settings.language) == 0)
					{
						/*preselection*/
						*sel_iter=iter;
					}
					
					xmlFree(url);
					xmlFree(name);
				}
				xmlXPathFreeObject (resultUrl);
			}
		}
		xmlXPathFreeObject (resultName);
	}
	
	/*free the document */
	xmlFreeDoc(doc);
	/*
	*Free the global variables that may
	*have been allocated by the parser.
	*/
	xmlCleanupParser();
	if(chunk.memory)
		free(chunk.memory);

	return GTK_TREE_MODEL (store);
}

  gboolean
  view_selection_func (GtkTreeSelection *selection,
                       GtkTreeModel     *model,
                       GtkTreePath      *path,
                       gboolean          path_currently_selected,
                       gpointer          userdata)
  {
    GtkTreeIter iter;
 
    if (gtk_tree_model_get_iter(model, &iter, path))
    {
		guchar *language;
      guchar *url;

		gtk_tree_model_get(model, &iter, COL_NAME, &language, -1);
		gtk_tree_model_get(model, &iter, COL_URL, &url, -1);
 
      if (!path_currently_selected)
      {
		  // safe selection
		   wpl_set_url(language, url);

	  }
 
      g_free(url);
    }
 
    return TRUE; /* allow selection state to change */
  }



GtkWidget *
create_view_and_model ()
{
  GtkCellRenderer     *renderer = NULL;
  GtkTreeModel        *model = NULL;
  GtkWidget           *view = NULL;
  GtkTreeSelection    *selection = NULL;
	GtkTreeIter		  sel_iter;

  view = gtk_tree_view_new ();
  gtk_widget_set_size_request(view, -1, 200);

  /* --- Column #1 --- */

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1,      
                                               "Sprache",  
                                               renderer,
                                               "text", COL_NAME,
                                               NULL);

  /* --- Column #2 --- */

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1,      
                                               "URL",  
                                               renderer,
                                               "text", COL_URL,
                                               NULL);

	model = getWikipediaLanguages(&sel_iter);

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
    gtk_tree_selection_set_select_function(selection, view_selection_func, NULL, NULL);

  	gtk_tree_selection_set_mode(selection,
                              GTK_SELECTION_SINGLE);

	gtk_tree_view_set_model (GTK_TREE_VIEW (view), model);

	/* preselection */
	gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(view)), &sel_iter);
	gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(view),
	                             gtk_tree_model_get_path(model, &sel_iter),
	                             NULL,
	                             TRUE,0.5,0);
	
  /* The tree view has acquired its own reference to the
   *  model, so we can drop ours. That way the model will
   *  be freed automatically when the tree view is destroyed */

  g_object_unref (model);

  return view;
}
