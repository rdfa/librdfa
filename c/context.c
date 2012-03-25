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

   // if the base isn't specified, don't create a context
   if(base_length > 0)
   {
      char* cleaned_base;
      rval = (rdfacontext*)malloc(sizeof(rdfacontext));
      rval->base = NULL;
      cleaned_base = rdfa_iri_get_base(base);
      rval->base = rdfa_replace_string(rval->base, cleaned_base);
      free(cleaned_base);

      // no callbacks set yet
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
   // assume the RDFa processing rules are RDFa 1.1 unless otherwise specified
   context->rdfa_version = RDFA_VERSION_1_1;

   // the [parent subject] is set to the [base] value;
   context->parent_subject = NULL;
   if(context->base != NULL)
   {
      char* cleaned_base = rdfa_iri_get_base(context->base);
      context->parent_subject =
         rdfa_replace_string(context->parent_subject, cleaned_base);
      free(cleaned_base);
   }

   // the [parent object] is set to null;
   context->parent_object = NULL;

#ifndef LIBRDFA_IN_RAPTOR
   // the [list of URI mappings] is cleared;
   context->uri_mappings = rdfa_create_mapping(MAX_URI_MAPPINGS);
#endif

   // the [list of incomplete triples] is cleared;
   context->incomplete_triples = rdfa_create_list(3);

   // the [language] is set to null.
   context->language = NULL;

   // set the [current object resource] to null;
   context->current_object_resource = NULL;

   // the list of term mappings is set to null
   // (or a list defined in the initial context of the Host Language).
   context->term_mappings = rdfa_create_mapping(MAX_TERM_MAPPINGS);

   // the maximum number of list mappings
   context->list_mappings = rdfa_create_mapping(MAX_LIST_MAPPINGS);

   // the maximum number of local list mappings
   context->local_list_mappings =
      rdfa_create_mapping(MAX_LOCAL_LIST_MAPPINGS);

   // the default vocabulary is set to null
   // (or a IRI defined in the initial context of the Host Language).
   context->default_vocabulary = NULL;

   // whether or not the @inlist attribute is present on the current element
   context->inlist_present = 0;

   // whether or not the @rel attribute is present on the current element
   context->rel_present = 0;

   // whether or not the @rev attribute is present on the current element
   context->rev_present = 0;

   // 1. First, the local values are initialized, as follows:
   //
   // * the [recurse] flag is set to 'true';
   context->recurse = 1;

   // * the [skip element] flag is set to 'false';
   context->skip_element = 0;

   // * [new subject] is set to null;
   context->new_subject = NULL;

   // * [current object resource] is set to null;
   context->current_object_resource = NULL;

   // * the [local list of URI mappings] is set to the list of URI
   //   mappings from the [evaluation context];
   //   NOTE: This step is done in rdfa_create_new_element_context()

   // FIXME: Initialize the term mappings and URI mappings based on Host Language

   // * the [local list of incomplete triples] is set to null;
   context->local_incomplete_triples = rdfa_create_list(3);

   // * the [current language] value is set to the [language] value
   //   from the [evaluation context].
   //   NOTE: This step is done in rdfa_create_new_element_context()

   // The next set of variables are initialized to make the C compiler
   // and valgrind happy - they are not a part of the RDFa spec.
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
   // FIXME: completing incomplete triples always happens now, change
   //        all of the code to reflect that.
   //context->callback_data = NULL;
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

   // * Otherwise, the values are:

   // * the [ base ] is set to the [ base ] value of the current
   //   [ evaluation context ];
   rval->base = rdfa_replace_string(rval->base, parent_context->base);
   rdfa_init_context(rval);

   // copy the URI mappings
#ifndef LIBRDFA_IN_RAPTOR
   rdfa_free_mapping(rval->uri_mappings, (free_mapping_value_fp)free);
   rval->uri_mappings =
      rdfa_copy_mapping((void**)parent_context->uri_mappings,
         (copy_mapping_value_fp)rdfa_replace_string);
#endif

   // inherit the parent context's RDFa processor mode
   rval->rdfa_version = parent_context->rdfa_version;

   // inherit the parent context's language
   if(parent_context->language != NULL)
   {
      rval->language =
         rdfa_replace_string(rval->language, parent_context->language);
   }

   // inherit the parent context's default vocabulary
   if(parent_context->default_vocabulary != NULL)
   {
      rval->default_vocabulary = rdfa_replace_string(
         rval->default_vocabulary, parent_context->default_vocabulary);
   }

   // set the callbacks callback
   rval->default_graph_triple_callback =
      parent_context->default_graph_triple_callback;
   rval->processor_graph_triple_callback =
      parent_context->processor_graph_triple_callback;
   rval->buffer_filler_callback = parent_context->buffer_filler_callback;

   // inherit the bnode count, _: bnode name, recurse flag, and state
   // of the xml_literal_namespace_insertion
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

   // inherit the parent context's new_subject
   // TODO: This is not anywhere in the syntax processing document
   //if(parent_context->new_subject != NULL)
   //{
   //   rval->new_subject = rdfa_replace_string(
   //      rval->new_subject, parent_context->new_subject);
   //}

   if(parent_context->skip_element == 0)
   {
      // o the [ parent subject ] is set to the value of [ new subject ],
      //   if non-null, or the value of the [ parent subject ] of the
      //   current [ evaluation context ];
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

      // o the [ parent object ] is set to value of [ current object
      //   resource ], if non-null, or the value of [ new subject ], if
      //   non-null, or the value of the [ parent subject ] of the
      //   current [ evaluation context ];
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

      // copy the incomplete triples
      if(rval->incomplete_triples != NULL)
      {
         rdfa_free_list(rval->incomplete_triples);
      }

      // o the [ list of incomplete triples ] is set to the [ local list
      //   of incomplete triples ];
      rval->incomplete_triples =
         rdfa_copy_list(parent_context->local_incomplete_triples);
   }
   else
   {
      rval->parent_subject = rdfa_replace_string(
         rval->parent_subject, parent_context->parent_subject);
      rval->parent_object = rdfa_replace_string(
         rval->parent_object, parent_context->parent_object);

      // copy the incomplete triples
      rdfa_free_list(rval->incomplete_triples);
      rval->incomplete_triples =
         rdfa_copy_list(parent_context->incomplete_triples);

      // copy the local list of incomplete triples
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
   // this field is not NULL only on the rdfacontext* at the top of the stack
   if(context->context_stack != NULL)
   {
      void* rval;
      // free the stack ensuring that we do not delete this context if
      // it is in the list (which it may be, if parsing ended on error)
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

   // TODO: These should be moved into their own data structure
   rdfa_free_list(context->local_incomplete_triples);

   rdfa_free_context_stack(context);
   free(context->working_buffer);
   free(context);
}
