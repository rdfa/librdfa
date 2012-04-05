/**
 * Copyright 2008-2011 Digital Bazaar, Inc.
 *
 * This file is part of librdfa.
 *
 * librdfa is Free Software, and can be licensed under any of the
 * following three licenses:
 *
 *   1. GNU Lesser General Public License (LGPL) V2.1 or any
 *      newer version
 *   2. GNU General Public License (GPL) V2 or any newer version
 *   3. Apache License, V2.0 or any newer version
 *
 * You may not use this file except in compliance with at least one of
 * the above three licenses.
 *
 * See LICENSE-* at the top of this software distribution for more
 * information regarding the details of each license.
 *
 * The librdfa library is the Fastest RDFa Parser in the Universe. It is
 * a stream parser, meaning that it takes an XML data as input and spits
 * out RDF triples as it comes across them in the stream. Due to this
 * processing approach, librdfa has a very, very small memory footprint.
 * It is also very fast and can operate on hundreds of gigabytes of XML
 * data without breaking a sweat.
 */
#include <string.h>
#include "rdfa_utils.h"

rdfacontext* rdfa_create_context(const char* base)
{
   rdfacontext* rval = NULL;
   size_t base_length = strlen(base);

   /* if the base isn't specified, don't create a context */
   if(base_length > 0)
   {
      char* cleaned_base;
      rval = (rdfacontext*)malloc(sizeof(rdfacontext));
      rval->base = NULL;
      rval->depth = 0;
      cleaned_base = rdfa_iri_get_base(base);
      rval->base = rdfa_replace_string(rval->base, cleaned_base);
      free(cleaned_base);

      /* no callbacks set yet */
      rval->default_graph_triple_callback = NULL;
      rval->buffer_filler_callback = NULL;
      rval->processor_graph_triple_callback = NULL;
      rval->callback_data = NULL;

      /* parse state */
      rval->wb_allocated = 0;
      rval->working_buffer = NULL;
      rval->wb_position = 0;
#ifdef LIBRDFA_IN_RAPTOR
      rval->base_uri = NULL;
      rval->sax2 = NULL;
      rval->namespace_handler = NULL;
      rval->namespace_handler_user_data = NULL;
#else
      rval->uri_mappings = NULL;
      rval->parser = NULL;
#endif
      rval->done = 0;
      rval->context_stack = NULL;
      rval->wb_preread = 0;
      rval->preread = 0;
   }
   else
   {
      printf("librdfa error: Failed to create a parsing context, "
         "base IRI was not specified!\n");
   }

   return rval;
}

void rdfa_init_context(rdfacontext* context)
{
   /* assume the RDFa processing rules are RDFa 1.1 unless otherwise specified */
   context->rdfa_version = RDFA_VERSION_1_1;

   /* assume the default host language is XML1 */
   context->host_language = HOST_LANGUAGE_XML1;

   /* the [parent subject] is set to the [base] value; */
   context->parent_subject = NULL;
   if(context->base != NULL)
   {
      char* cleaned_base = rdfa_iri_get_base(context->base);
      context->parent_subject =
         rdfa_replace_string(context->parent_subject, cleaned_base);
      free(cleaned_base);
   }

   /* the [parent object] is set to null; */
   context->parent_object = NULL;

#ifndef LIBRDFA_IN_RAPTOR
   /* the [list of URI mappings] is cleared; */
   context->uri_mappings = rdfa_create_mapping(MAX_URI_MAPPINGS);
#endif

   /* the [list of incomplete triples] is cleared; */
   context->incomplete_triples = rdfa_create_list(3);

   /* the [language] is set to null. */
   context->language = NULL;

   /* set the [current object resource] to null; */
   context->current_object_resource = NULL;

   /* the list of term mappings is set to null
    * (or a list defined in the initial context of the Host Language). */
   context->term_mappings = rdfa_create_mapping(MAX_TERM_MAPPINGS);

   /* the maximum number of list mappings */
   context->list_mappings = rdfa_create_mapping(MAX_LIST_MAPPINGS);

   /* the maximum number of local list mappings */
   context->local_list_mappings =
      rdfa_create_mapping(MAX_LOCAL_LIST_MAPPINGS);

   /* the default vocabulary is set to null
    * (or a IRI defined in the initial context of the Host Language). */
   context->default_vocabulary = NULL;

   /* whether or not the @inlist attribute is present on the current element */
   context->inlist_present = 0;

   /* whether or not the @rel attribute is present on the current element */
   context->rel_present = 0;

   /* whether or not the @rev attribute is present on the current element */
   context->rev_present = 0;

   /* 1. First, the local values are initialized, as follows:
    *
    * * the [recurse] flag is set to 'true'; */
   context->recurse = 1;

   /* * the [skip element] flag is set to 'false'; */
   context->skip_element = 0;

   /* * [new subject] is set to null; */
   context->new_subject = NULL;

   /* * [current object resource] is set to null; */
   context->current_object_resource = NULL;

   /* * the [local list of URI mappings] is set to the list of URI
    *   mappings from the [evaluation context];
    *   NOTE: This step is done in rdfa_create_new_element_context() */

   /* FIXME: Initialize the term mappings and URI mappings based on Host Language */

   /* * the [local list of incomplete triples] is set to null; */
   context->local_incomplete_triples = rdfa_create_list(3);

   /* * the [current language] value is set to the [language] value
    *   from the [evaluation context].
    *   NOTE: This step is done in rdfa_create_new_element_context() */

   /* The next set of variables are initialized to make the C compiler
    * and valgrind happy - they are not a part of the RDFa spec. */
   context->bnode_count = 0;
   context->underscore_colon_bnode_name = NULL;
   context->xml_literal_namespaces_defined = 0;
   context->xml_literal_xml_lang_defined = 0;

   context->about = NULL;
   context->typed_resource = NULL;
   context->resource = NULL;
   context->href = NULL;
   context->src = NULL;
   context->content = NULL;
   context->datatype = NULL;
   context->property = NULL;
   context->plain_literal = NULL;
   context->plain_literal_size = 0;
   context->xml_literal = NULL;
   context->xml_literal_size = 0;
   /* FIXME: completing incomplete triples always happens now, change
    *        all of the code to reflect that. */
   /*context->callback_data = NULL;*/
}

rdfa_setup_initial_context(rdfacontext* context)
{
   /* Setup the base RDFa 1.1 prefix and term mappings */
   if(context->rdfa_version == RDFA_VERSION_1_1)
   {
      /* Setup the base RDFa 1.1 prefix mappings */
      rdfa_update_mapping(context->uri_mappings,
         "grddl", "http://www.w3.org/2003/g/data-view#",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->uri_mappings,
         "ma", "http://www.w3.org/ns/ma-ont#",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->uri_mappings,
         "owl", "http://www.w3.org/2002/07/owl#",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->uri_mappings,
         "rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->uri_mappings,
         "rdfa", "http://www.w3.org/ns/rdfa#",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->uri_mappings,
         "rdfs", "http://www.w3.org/2000/01/rdf-schema#",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->uri_mappings,
         "rif", "http://www.w3.org/2007/rif#",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->uri_mappings,
         "skos", "http://www.w3.org/2004/02/skos/core#",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->uri_mappings,
         "skosxl", "http://www.w3.org/2008/05/skos-xl#",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->uri_mappings,
         "wdr", "http://www.w3.org/2007/05/powder#",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->uri_mappings,
         "void", "http://rdfs.org/ns/void#",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->uri_mappings,
         "wdrs", "http://www.w3.org/2007/05/powder-s#",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->uri_mappings,
         "xhv", "http://www.w3.org/1999/xhtml/vocab#",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->uri_mappings,
         "xml", "http://www.w3.org/XML/1998/namespace",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->uri_mappings,
         "xsd", "http://www.w3.org/2001/XMLSchema#",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->uri_mappings,
         "cc", "http://creativecommons.org/ns#",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->uri_mappings,
         "ctag", "http://commontag.org/ns#",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->uri_mappings,
         "dc", "http://purl.org/dc/terms/",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->uri_mappings,
         "dcterms", "http://purl.org/dc/terms/",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->uri_mappings,
         "foaf", "http://xmlns.com/foaf/0.1/",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->uri_mappings,
         "gr", "http://purl.org/goodrelations/v1#",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->uri_mappings,
         "ical", "http://www.w3.org/2002/12/cal/icaltzd#",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->uri_mappings,
         "og", "http://ogp.me/ns#",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->uri_mappings,
         "rev", "http://purl.org/stuff/rev#",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->uri_mappings,
         "sioc", "http://rdfs.org/sioc/ns#",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->uri_mappings,
         "v", "http://rdf.data-vocabulary.org/#",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->uri_mappings,
         "vcard", "http://www.w3.org/2006/vcard/ns#",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->uri_mappings,
         "schema", "http://schema.org/",
         (update_mapping_value_fp)rdfa_replace_string);

      /* Setup the base RDFa 1.1 term mappings */
      rdfa_update_mapping(context->term_mappings,
         "describedby", "http://www.w3.org/2007/05/powder-s#describedby",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "license", "http://www.w3.org/1999/xhtml/vocab#license",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "role", "http://www.w3.org/1999/xhtml/vocab#role",
         (update_mapping_value_fp)rdfa_replace_string);
   }

   /* Setup the term mappings for XHTML1 */
   if(context->host_language == HOST_LANGUAGE_XHTML1)
   {
      rdfa_update_mapping(context->term_mappings,
         "alternate", "http://www.w3.org/1999/xhtml/vocab#alternate",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "appendix", "http://www.w3.org/1999/xhtml/vocab#appendix",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "cite", "http://www.w3.org/1999/xhtml/vocab#cite",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "bookmark", "http://www.w3.org/1999/xhtml/vocab#bookmark",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "contents", "http://www.w3.org/1999/xhtml/vocab#contents",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "chapter", "http://www.w3.org/1999/xhtml/vocab#chapter",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "copyright", "http://www.w3.org/1999/xhtml/vocab#copyright",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "first", "http://www.w3.org/1999/xhtml/vocab#first",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "glossary", "http://www.w3.org/1999/xhtml/vocab#glossary",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "help", "http://www.w3.org/1999/xhtml/vocab#help",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "icon", "http://www.w3.org/1999/xhtml/vocab#icon",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "index", "http://www.w3.org/1999/xhtml/vocab#index",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "last", "http://www.w3.org/1999/xhtml/vocab#last",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "license", "http://www.w3.org/1999/xhtml/vocab#license",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "meta", "http://www.w3.org/1999/xhtml/vocab#meta",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "next", "http://www.w3.org/1999/xhtml/vocab#next",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "prev", "http://www.w3.org/1999/xhtml/vocab#prev",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "previous", "http://www.w3.org/1999/xhtml/vocab#previous",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "section", "http://www.w3.org/1999/xhtml/vocab#section",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "start", "http://www.w3.org/1999/xhtml/vocab#start",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "stylesheet", "http://www.w3.org/1999/xhtml/vocab#stylesheet",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "subsection", "http://www.w3.org/1999/xhtml/vocab#subsection",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "top", "http://www.w3.org/1999/xhtml/vocab#top",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "up", "http://www.w3.org/1999/xhtml/vocab#up",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "p3pv1", "http://www.w3.org/1999/xhtml/vocab#p3pv1",
         (update_mapping_value_fp)rdfa_replace_string);

      /* From the role attribute module */
      rdfa_update_mapping(context->term_mappings,
         "role", "http://www.w3.org/1999/xhtml/vocab#role",
         (update_mapping_value_fp)rdfa_replace_string);
   }

   /* Setup the prefix and term mappings for HTML4 and HTML5 */
   if(context->host_language == HOST_LANGUAGE_HTML)
   {
      /* No term or prefix mappings as of 2012-04-04 */
   }

   /* Generate namespace triples for all values in the uri_mapping */
   char* key = NULL;
   char* value = NULL;
   void** mptr = context->uri_mappings;
   while(*mptr != NULL)
   {
      rdfa_next_mapping(mptr++, &key, &value);
      mptr++;
      rdfa_generate_namespace_triple(context, key, value);
   }
}

/**
 * Creates a new context for the current element by cloning certain
 * parts of the old context on the top of the given stack.
 *
 * @param context_stack the context stack that is associated with this
 *                      processing run.
 */
rdfacontext* rdfa_create_new_element_context(rdfalist* context_stack)
{
   rdfacontext* parent_context = (rdfacontext*)
      context_stack->items[context_stack->num_items - 1]->data;
   rdfacontext* rval = rdfa_create_context(parent_context->base);

   /* * Otherwise, the values are: */

   /* * the [ base ] is set to the [ base ] value of the current
    *   [ evaluation context ]; */
   rval->base = rdfa_replace_string(rval->base, parent_context->base);
   rdfa_init_context(rval);

   /* Set the processing depth as parent + 1 */
   rval->depth = parent_context->depth + 1;

   /* copy the URI mappings */
#ifndef LIBRDFA_IN_RAPTOR
   rdfa_free_mapping(rval->uri_mappings, (free_mapping_value_fp)free);
   rdfa_free_mapping(rval->term_mappings, (free_mapping_value_fp)free);
   rdfa_free_mapping(rval->list_mappings, (free_mapping_value_fp)rdfa_free_list);
   rdfa_free_mapping(rval->local_list_mappings, (free_mapping_value_fp)rdfa_free_list);
   rval->uri_mappings =
      rdfa_copy_mapping((void**)parent_context->uri_mappings,
         (copy_mapping_value_fp)rdfa_replace_string);
   rval->term_mappings =
      rdfa_copy_mapping((void**)parent_context->term_mappings,
         (copy_mapping_value_fp)rdfa_replace_string);
   rval->list_mappings =
      rdfa_copy_mapping((void**)parent_context->local_list_mappings,
         (copy_mapping_value_fp)rdfa_replace_list);
   rval->local_list_mappings =
      rdfa_copy_mapping((void**)parent_context->local_list_mappings,
         (copy_mapping_value_fp)rdfa_replace_list);
#endif

   /* inherit the parent context's host language and RDFa processor mode */
   rval->host_language = parent_context->host_language;
   rval->rdfa_version = parent_context->rdfa_version;

   /* inherit the parent context's language */
   if(parent_context->language != NULL)
   {
      rval->language =
         rdfa_replace_string(rval->language, parent_context->language);
   }

   /* inherit the parent context's default vocabulary */
   if(parent_context->default_vocabulary != NULL)
   {
      rval->default_vocabulary = rdfa_replace_string(
         rval->default_vocabulary, parent_context->default_vocabulary);
   }

   /* set the callbacks callback */
   rval->default_graph_triple_callback =
      parent_context->default_graph_triple_callback;
   rval->processor_graph_triple_callback =
      parent_context->processor_graph_triple_callback;
   rval->buffer_filler_callback = parent_context->buffer_filler_callback;

   /* inherit the bnode count, _: bnode name, recurse flag, and state
    * of the xml_literal_namespace_insertion */
   rval->bnode_count = parent_context->bnode_count;
   rval->underscore_colon_bnode_name =
      rdfa_replace_string(rval->underscore_colon_bnode_name,
                          parent_context->underscore_colon_bnode_name);
   rval->recurse = parent_context->recurse;
   rval->skip_element = 0;
   rval->callback_data = parent_context->callback_data;
   rval->xml_literal_namespaces_defined =
      parent_context->xml_literal_namespaces_defined;
   rval->xml_literal_xml_lang_defined =
      parent_context->xml_literal_xml_lang_defined;

#if 0
   /* inherit the parent context's new_subject
    * TODO: This is not anywhere in the syntax processing document */
   if(parent_context->new_subject != NULL)
   {
      rval->new_subject = rdfa_replace_string(
         rval->new_subject, parent_context->new_subject);
   }
#endif

   if(parent_context->skip_element == 0)
   {
      /* o the [ parent subject ] is set to the value of [ new subject ],
       *   if non-null, or the value of the [ parent subject ] of the
       *   current [ evaluation context ]; */
      if(parent_context->new_subject != NULL)
      {
         rval->parent_subject = rdfa_replace_string(
            rval->parent_subject, parent_context->new_subject);
      }
      else
      {
         rval->parent_subject = rdfa_replace_string(
            rval->parent_subject, parent_context->parent_subject);
      }

      /* o the [ parent object ] is set to value of [ current object
       *   resource ], if non-null, or the value of [ new subject ], if
       *   non-null, or the value of the [ parent subject ] of the
       *   current [ evaluation context ]; */
      if(parent_context->current_object_resource != NULL)
      {
         rval->parent_object =
            rdfa_replace_string(
               rval->parent_object, parent_context->current_object_resource);
      }
      else if(parent_context->new_subject != NULL)
      {
         rval->parent_object =
            rdfa_replace_string(
               rval->parent_object, parent_context->new_subject);
      }
      else
      {
         rval->parent_object =
            rdfa_replace_string(
               rval->parent_object, parent_context->parent_subject);
      }

      /* copy the incomplete triples */
      if(rval->incomplete_triples != NULL)
      {
         rdfa_free_list(rval->incomplete_triples);
      }

      /* o the [ list of incomplete triples ] is set to the [ local list
       *   of incomplete triples ]; */
      rval->incomplete_triples =
         rdfa_copy_list(parent_context->local_incomplete_triples);
   }
   else
   {
      rval->parent_subject = rdfa_replace_string(
         rval->parent_subject, parent_context->parent_subject);
      rval->parent_object = rdfa_replace_string(
         rval->parent_object, parent_context->parent_object);

      /* copy the incomplete triples */
      rdfa_free_list(rval->incomplete_triples);
      rval->incomplete_triples =
         rdfa_copy_list(parent_context->incomplete_triples);

      /* copy the local list of incomplete triples */
      rdfa_free_list(rval->local_incomplete_triples);
      rval->local_incomplete_triples =
         rdfa_copy_list(parent_context->local_incomplete_triples);
   }

#ifdef LIBRDFA_IN_RAPTOR
   rval->base_uri = parent_context->base_uri;
   rval->sax2     = parent_context->sax2;
   rval->namespace_handler = parent_context->namespace_handler;
   rval->namespace_handler_user_data = parent_context->namespace_handler_user_data;
#endif

   return rval;
}

void rdfa_free_context_stack(rdfacontext* context)
{
   /* this field is not NULL only on the rdfacontext* at the top of the stack */
   if(context->context_stack != NULL)
   {
      void* rval;
      /* free the stack ensuring that we do not delete this context if
       * it is in the list (which it may be, if parsing ended on error) */
      do
      {
         rval = rdfa_pop_item(context->context_stack);
         if(rval && rval != context)
         {
            rdfa_free_context((rdfacontext*)rval);
         }
      }
      while(rval);
      free(context->context_stack->items);
      free(context->context_stack);
      context->context_stack = NULL;
   }
}

void rdfa_free_context(rdfacontext* context)
{
   free(context->base);
   free(context->parent_subject);
   free(context->parent_object);

#ifndef LIBRDFA_IN_RAPTOR
   rdfa_free_mapping(context->uri_mappings, (free_mapping_value_fp)free);
#endif

   rdfa_free_mapping(context->term_mappings, (free_mapping_value_fp)free);
   rdfa_free_list(context->incomplete_triples);
   free(context->language);
   free(context->underscore_colon_bnode_name);
   free(context->new_subject);
   free(context->current_object_resource);
   free(context->about);
   free(context->typed_resource);
   free(context->resource);
   free(context->href);
   free(context->src);
   free(context->content);
   free(context->datatype);
   rdfa_free_list(context->property);
   free(context->plain_literal);
   free(context->xml_literal);

   /* TODO: These should be moved into their own data structure */
   rdfa_free_list(context->local_incomplete_triples);

   rdfa_free_context_stack(context);
   free(context->working_buffer);
   free(context);
}
