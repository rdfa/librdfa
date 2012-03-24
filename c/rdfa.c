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
 *
 * Usage:
 *
 *    rdfacontext* context = rdfa_create_context(BASE_URI);
 *    context->callback_data = your_user_data;
 *    rdfa_set_default_graph_triple_handler(context, &default_graph_triple);
 *    rdfa_set_processor_graph_triple_handler(context, &processor_graph_triple);
 *    rdfa_set_buffer_filler(context, &fill_buffer);
 *    rdfa_parse(context);
 *    rdfa_free_context(context);
 *
 * @author Manu Sporny
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <libxml/SAX2.h>
#include "rdfa_utils.h"
#include "rdfa.h"

#define READ_BUFFER_SIZE 4096
#define RDFA_DOCTYPE_STRING_LENGTH 103

/**
 * Read the head of the XHTML document and determines the base IRI for
 * the document.
 *
 * @param context the current working context.
 * @param working_buffer the current working buffer.
 * @param wb_allocated the number of bytes that have been allocated to
 *                     the working buffer.
 *
 * @return the size of the data available in the working buffer.
 */
static size_t rdfa_init_base(
   rdfacontext* context, char** working_buffer, size_t* working_buffer_size,
   char* temp_buffer, size_t bytes_read)
{
   char* head_end = NULL;
   size_t offset = context->wb_position;
   size_t needed_size = 0;

   if((offset + bytes_read) > *working_buffer_size)
   {
      needed_size = (offset + bytes_read) - *working_buffer_size;
   }

   // search for the end of <head>, stop if <head> was found

   // extend the working buffer size
   if(needed_size > 0)
   {
      size_t temp_buffer_size = sizeof(char) * READ_BUFFER_SIZE;
      if((size_t)needed_size > temp_buffer_size)
         temp_buffer_size += needed_size;

      *working_buffer_size += temp_buffer_size;
      // +1 for NUL at end, to allow strstr() etc. to work
      *working_buffer = (char*)realloc(*working_buffer, *working_buffer_size + 1);
   }

   // append to the working buffer
   memmove(*working_buffer + offset, temp_buffer, bytes_read);
   // ensure the buffer is a NUL-terminated string
   *(*working_buffer + offset + bytes_read) = '\0';

   // search for an RDFa 1.0 DOCTYPE string to set the version
   if(strstr(*working_buffer, "-//W3C//DTD XHTML+RDFa 1.0//EN") != NULL)
   {
      context->rdfa_version = RDFA_VERSION_1_0;
   }

   // search for the end of </head> in
   head_end = strstr(*working_buffer, "</head>");
   if(head_end == NULL)
      head_end = strstr(*working_buffer, "</HEAD>");

   context->wb_position += bytes_read;

   if(head_end == NULL)
      return bytes_read;

   // if </head> was found, search for <base and extract the base URI
   if(head_end != NULL)
   {
      char* base_start = strstr(*working_buffer, "<base ");
      if(base_start == NULL)
         base_start = strstr(*working_buffer, "<BASE ");

      if(base_start != NULL)
      {
         char* href_start = strstr(base_start, "href=");
         char sep = href_start[5];
         char* uri_start = href_start + 6;
         char* uri_end = strchr(uri_start, sep);

         if((uri_start != NULL) && (uri_end != NULL))
         {
            if(*uri_start != sep)
            {
               size_t uri_size = uri_end - uri_start;
               char* temp_uri = (char*)malloc(sizeof(char) * uri_size + 1);
	       char* cleaned_base;
               strncpy(temp_uri, uri_start, uri_size);
               temp_uri[uri_size] = '\0';

               // TODO: This isn't in the processing rules, should it
               //       be? Setting current_object_resource will make
               //       sure that the BASE element is inherited by all
               //       subcontexts.
	       cleaned_base = rdfa_iri_get_base(temp_uri);
               context->current_object_resource =
                  rdfa_replace_string(
                     context->current_object_resource, cleaned_base);

	       // clean up the base context
               context->base =
                  rdfa_replace_string(context->base, cleaned_base);
               free(cleaned_base);
               free(temp_uri);
            }
         }
      }
   }

   return bytes_read;
}

/**
 * Handles the start_element call
 */
static void start_element(void *parser_context, const char* name,
   const char* prefix, const char* URI, int nb_namespaces,
   const char** namespaces, int nb_attributes, int nb_defaulted,
   const char** attributes)
{
   rdfacontext* root_context = (rdfacontext*)parser_context;
   rdfalist* context_stack = (rdfalist*)root_context->context_stack;
   rdfacontext* context = rdfa_create_new_element_context(context_stack);
   char* xml_lang = NULL;
   const char* about_curie = NULL;
   char* about = NULL;
   const char* src_curie = NULL;
   char* src = NULL;
   const char* type_of_curie = NULL;
   rdfalist* type_of = NULL;
   unsigned char inlist = 0;
   const char* rel_curie = NULL;
   rdfalist* rel = NULL;
   const char* rev_curie = NULL;
   rdfalist* rev = NULL;
   const char* property_curie = NULL;
   rdfalist* property = NULL;
   const char* resource_curie = NULL;
   char* resource = NULL;
   const char* href_curie = NULL;
   char* href = NULL;
   char* content = NULL;
   const char* datatype_curie = NULL;
   char* datatype = NULL;

   rdfa_push_item(context_stack, context, RDFALIST_FLAG_CONTEXT);

   if(DEBUG)
   {
      int i;

      // dump all arguments sent to this callback
      fprintf(stdout, "SAX.startElementNs(%s", (char *) name);
      if (prefix == NULL)
          fprintf(stdout, ", NULL");
      else
          fprintf(stdout, ", %s", (char *) prefix);
      if (URI == NULL)
          fprintf(stdout, ", NULL");
      else
          fprintf(stdout, ", '%s'", (char *) URI);
      fprintf(stdout, ", %d", nb_namespaces);

      // dump all namespaces
      if (namespaces != NULL) {
          for (i = 0;i < nb_namespaces * 2;i++) {
              fprintf(stdout, ", xmlns");
              if (namespaces[i] != NULL)
                  fprintf(stdout, ":%s", namespaces[i]);
              i++;
              fprintf(stdout, "='%s'", namespaces[i]);
          }
      }

      // dump all attributes
      fprintf(stdout, ", %d, %d", nb_attributes, nb_defaulted);
      if (attributes != NULL) {
          for (i = 0;i < nb_attributes * 5;i += 5) {
              if (attributes[i + 1] != NULL)
                  fprintf(
                     stdout, ", %s:%s='", attributes[i + 1], attributes[i]);
              else
                  fprintf(stdout, ", %s='", attributes[i]);
              fprintf(stdout, "%.4s...', %d", attributes[i + 3],
                      (int)(attributes[i + 4] - attributes[i + 3]));
          }
      }
      fprintf(stdout, ")\n");
   }

   // start the XML Literal text
   if(context->xml_literal == NULL)
   {
      context->xml_literal = rdfa_replace_string(context->xml_literal, "<");
      context->xml_literal_size = 1;
   }
   else
   {
      context->xml_literal = rdfa_n_append_string(
         context->xml_literal, &context->xml_literal_size, "<", 1);
   }
   context->xml_literal = rdfa_n_append_string(
      context->xml_literal, &context->xml_literal_size,
      name, strlen(name));

   if(!context->xml_literal_namespaces_defined)
   {
      // append namespaces to XML Literal
#ifdef LIBRDFA_IN_RAPTOR
      raptor_namespace_stack* nstack = &context->sax2->namespaces;
      raptor_namespace* ns;
      raptor_namespace** ns_list = NULL;
      size_t ns_size;
#else
      void** umap = context->uri_mappings;
#endif
      char* umap_key = NULL;
      void* umap_value = NULL;

      // if the namespaces are not defined, then neither is the xml:lang
      context->xml_literal_xml_lang_defined = 0;

#ifdef LIBRDFA_IN_RAPTOR
      ns_size = 0;
      ns_list = raptor_namespace_stack_to_array(nstack, &ns_size);
      qsort((void*)ns_list, ns_size, sizeof(raptor_namespace*),
            raptor_nspace_compare);

      while(ns_size > 0)
#else
      while(*umap != NULL)
#endif
      {
         unsigned char insert_xmlns_definition = 1;
         const char* attr = NULL;

         // get the next mapping to process
#ifdef LIBRDFA_IN_RAPTOR
         ns=ns_list[--ns_size];

         umap_key = (char*)raptor_namespace_get_prefix(ns);
         if(!umap_key)
           umap_key=(char*)XMLNS_DEFAULT_MAPPING;
         umap_value = (char*)raptor_uri_as_string(raptor_namespace_get_uri(ns));
#else
         rdfa_next_mapping(umap++, &umap_key, &umap_value);
         umap++;
#endif

         // check to make sure that the namespace isn't already
         // defined in the current element.
         if(attributes != NULL)
         {
            const char** attrs = attributes;
            while((*attrs != NULL) && insert_xmlns_definition)
            {
               attr = *attrs++;

               // if the attribute is a umap_key, skip the definition
               // of the attribute.
               if(strcmp(attr, umap_key) == 0)
               {
                  insert_xmlns_definition = 0;
               }
            }
         }

         // if the namespace isn't already defined on the element,
         // copy it to the XML Literal string.
         if(insert_xmlns_definition)
         {
            // append the namespace attribute to the XML Literal
            context->xml_literal = rdfa_n_append_string(
               context->xml_literal, &context->xml_literal_size,
               " xmlns", strlen(" xmlns"));

            // check to see if we're dumping the standard XHTML namespace or
            // a user-defined XML namespace
            if(strcmp(umap_key, XMLNS_DEFAULT_MAPPING) != 0)
            {
               context->xml_literal = rdfa_n_append_string(
                  context->xml_literal, &context->xml_literal_size, ":", 1);
               context->xml_literal = rdfa_n_append_string(
                  context->xml_literal, &context->xml_literal_size,
                  umap_key, strlen(umap_key));
            }

            // append the namespace value
            context->xml_literal = rdfa_n_append_string(
               context->xml_literal, &context->xml_literal_size, "=\"", 2);
            context->xml_literal = rdfa_n_append_string(
               context->xml_literal, &context->xml_literal_size,
               umap_value, strlen((char*)umap_value));
            context->xml_literal = rdfa_n_append_string(
               context->xml_literal, &context->xml_literal_size, "\"", 1);
         }

      } /* end while umap not NULL */
      context->xml_literal_namespaces_defined = 1;

#ifdef LIBRDFA_IN_RAPTOR
      if(ns_list)
        raptor_free_memory(ns_list);
#endif
   } /* end if namespaces inserted */

   // 3. For backward compatibility, RDFa Processors should also permit the
   // definition of mappings via @xmlns. In this case, the value to be mapped
   // is set by the XML namespace prefix, and the value to map is the value of
   // the attribute - an IRI. (Note that prefix mapping via @xmlns is
   // deprecated, and may be removed in a future version of this
   // specification.) When xmlns is supported, such mappings must be processed
   // before processing any mappings from @prefix on the same element.
   if(namespaces != NULL)
   {
      int ni;

      for(ni = 0; ni < nb_namespaces * 2; ni += 2)
      {
         const char* ns = namespaces[ni];
         const char* value = namespaces[ni + 1];
         // Regardless of how the mapping is declared, the value to be mapped
         // must be converted to lower case, and the IRI is not processed in
         // any way; in particular if it is a relative path it must not be
         // resolved against the current base.
         char* lcns = NULL;
         if(ns != NULL)
         {
            // convert the namespace string to lowercase
            int i;
            int ns_length = strlen(ns);
            lcns = (char*)malloc(ns_length + 1);
            for(i = 0; i <= ns_length; i++)
            {
               lcns[i] = tolower(ns[i]);
            }
         }

         // update the URI mappings
         rdfa_update_uri_mappings(context, lcns, value);

         if(lcns != NULL)
         {
            free(lcns);
         }
      }
   }

   // detect the RDFa version of the document, if specified
   if(attributes != NULL)
   {
      int ci;

      // search for a version attribute
      for(ci = 0; ci < nb_attributes * 5; ci += 5)
      {
         const char* attr;
         char* value;
         unsigned int value_length = 0;

         attr = attributes[ci];
         value_length = attributes[ci + 4] - attributes[ci + 3] + 1;

         if(strcmp(attr, "version") == 0)
         {
            // append the attribute-value pair to the XML literal
            value = (char*)malloc(value_length + 1);
            snprintf(value, value_length, "%s", attributes[ci + 3]);
            if(strstr(value, "RDFa 1.0") != NULL)
            {
               context->rdfa_version = RDFA_VERSION_1_0;
            }
            else if(strstr(value, "RDFa 1.1") != NULL)
            {
               context->rdfa_version = RDFA_VERSION_1_1;
            }

            free(value);
         }
      }
   }

   // prepare all of the RDFa-specific attributes we are looking for.
   // scan all of the attributes for the RDFa-specific attributes
   if(attributes != NULL)
   {
      int ci;

      if(context->rdfa_version == RDFA_VERSION_1_1)
      {
         // process all vocab and prefix attributes
         for(ci = 0; ci < nb_attributes * 5; ci += 5)
         {
            const char* attr;
            char* value;
            unsigned int value_length = 0;

            attr = attributes[ci];
            value_length = attributes[ci + 4] - attributes[ci + 3] + 1;

            // append the attribute-value pair to the XML literal
            value = (char*)malloc(value_length + 1);
            snprintf(value, value_length, "%s", attributes[ci + 3]);

            // 2. Next the current element is examined for any change to the
            // default vocabulary via @vocab.
            if(strcmp(attr, "vocab") == 0)
            {
               if(strlen(value) < 1)
               {
                  // If the value is empty, then the local default vocabulary
                  // must be reset to the Host Language defined default
                  // (if any).
                  free(context->default_vocabulary);
                  context->default_vocabulary = NULL;
               }
               else
               {
                  // If @vocab is present and contains a value, the local
                  // default vocabulary is updated according to the
                  // section on CURIE and IRI Processing.
                  char* resolved_uri = rdfa_resolve_uri(context, value);
                  context->default_vocabulary = rdfa_replace_string(
                     context->default_vocabulary, resolved_uri);

                  // The value of @vocab is used to generate a triple
                  rdftriple* triple = rdfa_create_triple(
                     context->base, "http://www.w3.org/ns/rdfa#usesVocabulary",
                     resolved_uri, RDF_TYPE_IRI, NULL, NULL);
                  context->default_graph_triple_callback(
                     triple, context->callback_data);

                  free(resolved_uri);
               }
            }
            else if(strcmp(attr, "prefix") == 0)
            {
               // Mappings are defined via @prefix.
               char* working_string = NULL;
               char* prefix = NULL;
               char* iri = NULL;
               char* saveptr = NULL;

               working_string = rdfa_replace_string(working_string, value);

               // Values in this attribute are evaluated from beginning to
               // end (e.g., left to right in typical documents).
               prefix = strtok_r(working_string, ":", &saveptr);
               while(prefix != NULL)
               {
                  // find the prefix and IRI mappings while skipping whitespace
                  while((*saveptr == ' ' || *saveptr == '\n' ||
                     *saveptr == '\r' || *saveptr == '\t' || *saveptr == '\f' ||
                     *saveptr == '\v') && *saveptr != '\0')
                  {
                     saveptr++;
                  }
                  iri = strtok_r(NULL, RDFA_WHITESPACE, &saveptr);
                  while((*saveptr == ' ' || *saveptr == '\n' ||
                     *saveptr == '\r' || *saveptr == '\t' || *saveptr == '\f' ||
                     *saveptr == '\v') && *saveptr != '\0')
                  {
                     saveptr++;
                  }

                  // update the prefix mappings
                  rdfa_update_uri_mappings(context, prefix, iri);

                  // get the next prefix to process
                  prefix = strtok_r(NULL, ":", &saveptr);
               }

               free(working_string);
            }
            else if(strcmp(attr, "inlist") == 0)
            {
               inlist = 1;
            }
            free(value);
         }
      }

      // resolve all of the other RDFa values
      for(ci = 0; ci < nb_attributes * 5; ci += 5)
      {
         const char* attr;
         char* value;
         char* attrns;
         char* literal_text;
         unsigned int value_length = 0;

         attr = attributes[ci];
         attrns = (char*)attributes[ci + 1];
         value_length = attributes[ci + 4] - attributes[ci + 3] + 1;

         // append the attribute-value pair to the XML literal
         value = (char*)malloc(value_length + 1);
         literal_text = (char*)malloc(strlen(attr) + value_length + 5);
         snprintf(value, value_length, "%s", attributes[ci + 3]);

         sprintf(literal_text, " %s=\"%s\"", attr, value);
         context->xml_literal = rdfa_n_append_string(
            context->xml_literal, &context->xml_literal_size,
            literal_text, strlen(literal_text));
         free(literal_text);

         // if xml:lang is defined, ensure that it is not overwritten
         if(attrns != NULL && strcmp(attrns, "xml") == 0 &&
            strcmp(attr, "lang") == 0)
         {
            context->xml_literal_xml_lang_defined = 1;
         }

         // process all of the RDFa attributes
         if(strcmp(attr, "about") == 0)
         {
            about_curie = value;
            about = rdfa_resolve_curie(
               context, about_curie, CURIE_PARSE_ABOUT_RESOURCE);
         }
         else if(strcmp(attr, "src") == 0)
         {
            src_curie = value;
            src = rdfa_resolve_curie(context, src_curie, CURIE_PARSE_HREF_SRC);
         }
         else if(strcmp(attr, "typeof") == 0)
         {
            type_of_curie = value;
            type_of = rdfa_resolve_curie_list(
               context, type_of_curie,
               CURIE_PARSE_INSTANCEOF_DATATYPE);
         }
         else if(strcmp(attr, "rel") == 0)
         {
            rel_curie = value;
            rel = rdfa_resolve_curie_list(
               context, rel_curie, CURIE_PARSE_RELREV);
         }
         else if(strcmp(attr, "rev") == 0)
         {
            rev_curie = value;
            rev = rdfa_resolve_curie_list(
               context, rev_curie, CURIE_PARSE_RELREV);
         }
         else if(strcmp(attr, "property") == 0)
         {
            property_curie = value;
            property =
               rdfa_resolve_curie_list(
                  context, property_curie, CURIE_PARSE_PROPERTY);
         }
         else if(strcmp(attr, "resource") == 0)
         {
            resource_curie = value;
            resource = rdfa_resolve_curie(
               context, resource_curie, CURIE_PARSE_ABOUT_RESOURCE);
         }
         else if(strcmp(attr, "href") == 0)
         {
            href_curie = value;
            href =
               rdfa_resolve_curie(context, href_curie, CURIE_PARSE_HREF_SRC);
         }
         else if(strcmp(attr, "content") == 0)
         {
            content = rdfa_replace_string(content, value);
         }
         else if(strcmp(attr, "datatype") == 0)
         {
            datatype_curie = value;

            if(strlen(datatype_curie) == 0)
            {
               datatype = rdfa_replace_string(datatype, "");
            }
            else
            {
               datatype = rdfa_resolve_curie(context, datatype_curie,
                  CURIE_PARSE_INSTANCEOF_DATATYPE);
            }
         }
#ifndef LIBRDFA_IN_RAPTOR
         else if((attrns == NULL && strcmp(attr, "lang") == 0) ||
            (attrns != NULL && strcmp(attrns, "xml") == 0 &&
               strcmp(attr, "lang") == 0))
         {
            xml_lang = rdfa_replace_string(xml_lang, value);
         }
#endif

         free(value);
      }
   }

#ifdef LIBRDFA_IN_RAPTOR
   if(context->sax2) {
      xml_lang = (const char*)raptor_sax2_inscope_xml_language(context->sax2);
      if(!xml_lang)
        xml_lang = "";
   }
#endif
   // check to see if we should append an xml:lang to the XML Literal
   // if one is defined in the context and does not exist on the
   // element.
   if((xml_lang == NULL) && (context->language != NULL) &&
      !context->xml_literal_xml_lang_defined)
   {
      context->xml_literal = rdfa_n_append_string(
         context->xml_literal, &context->xml_literal_size,
         " xml:lang=\"", strlen(" xml:lang=\""));
      context->xml_literal = rdfa_n_append_string(
         context->xml_literal, &context->xml_literal_size,
         context->language, strlen(context->language));
      context->xml_literal = rdfa_n_append_string(
         context->xml_literal, &context->xml_literal_size, "\"", 1);

      // ensure that the lang isn't set in a subtree (unless it's overwritten)
      context->xml_literal_xml_lang_defined = 1;
   }

   // close the XML Literal value
   context->xml_literal = rdfa_n_append_string(
      context->xml_literal, &context->xml_literal_size, ">", 1);

   // 3. The [current element] is also parsed for any language
   //    information, and [language] is set in the [current
   //    evaluation context];
   rdfa_update_language(context, xml_lang);

   /***************** FOR DEBUGGING PURPOSES ONLY ******************/
   if(DEBUG)
   {
      if(about != NULL)
      {
         printf("DEBUG: @about = %s\n", about);
      }
      if(src != NULL)
      {
         printf("DEBUG: @src = %s\n", src);
      }
      if(type_of != NULL)
      {
         printf("DEBUG: @type_of = ");
         rdfa_print_list(type_of);
      }
      if(inlist)
      {
         printf("DEBUG: @inlist = true\n");
      }
      if(rel != NULL)
      {
         printf("DEBUG: @rel = ");
         rdfa_print_list(rel);
      }
      if(rev != NULL)
      {
         printf("DEBUG: @rev = ");
         rdfa_print_list(rev);
      }
      if(property != NULL)
      {
         printf("DEBUG: @property = ");
         rdfa_print_list(property);
      }
      if(resource != NULL)
      {
         printf("DEBUG: @resource = %s\n", resource);
      }
      if(href != NULL)
      {
         printf("DEBUG: @href = %s\n", href);
      }
      if(content != NULL)
      {
         printf("DEBUG: @content = %s\n", content);
      }
      if(datatype != NULL)
      {
         printf("DEBUG: @datatype = %s\n", datatype);
      }
      if(xml_lang != NULL)
      {
         printf("DEBUG: @xml:lang = %s\n", xml_lang);
      }
   }

   // TODO: This isn't part of the processing model, it needs to be
   // included and is a correction for the last item in step #4.
   if((about == NULL) && (src == NULL) && (type_of == NULL) &&
      (rel == NULL) && (rev == NULL) && (property == NULL) &&
      (resource == NULL) && (href == NULL))
   {
      context->skip_element = 1;
   }

   if((rel == NULL) && (rev == NULL))
   {
      // 4. If the [current element] contains no valid @rel or @rev
      // URI, obtained according to the section on CURIE and URI
      // Processing, then the next step is to establish a value for
      // [new subject]. Any of the attributes that can carry a
      // resource can set [new subject];
      rdfa_establish_new_subject(
         context, name, about, src, resource, href, type_of);
   }
   else
   {
      // 5. If the [current element] does contain a valid @rel or @rev
      // URI, obtained according to the section on CURIE and URI
      // Processing, then the next step is to establish both a value
      // for [new subject] and a value for [current object resource]:
      rdfa_establish_new_subject_with_relrev(
         context, name, about, src, resource, href, type_of);
   }

   if(context->new_subject != NULL)
   {
      if(DEBUG)
      {
         printf("DEBUG: new_subject = %s\n", context->new_subject);
      }

      // 6. If in any of the previous steps a [new subject] was set to
      // a non-null value,

      // it is now used to provide a subject for type values;
      if(type_of != NULL)
      {
         rdfa_complete_type_triples(context, type_of);
      }

      // Note that none of this block is executed if there is no
      // [new subject] value, i.e., [new subject] remains null.
   }

   if(context->current_object_resource != NULL)
   {
      // 7. If in any of the previous steps a [current object  resource]
      // was set to a non-null value, it is now used to generate triples
      rdfa_complete_relrev_triples(context, rel, rev);
   }
   else if((rel != NULL) || (rev != NULL))
   {
      // 8. If however [current object resource] was set to null, but
      // there are predicates present, then they must be stored as
      // [incomplete triple]s, pending the discovery of a subject that
      // can be used as the object. Also, [current object resource]
      // should be set to a newly created [bnode]
      rdfa_save_incomplete_triples(context, rel, rev);
   }

   // Ensure to re-insert XML Literal namespace information from this
   // point on...
   if(property != NULL)
   {
      context->xml_literal_namespaces_defined = 0;
   }

   // save these for processing steps #9 and #10
   context->property = property;
   context->content = rdfa_replace_string(context->datatype, content);
   context->datatype = rdfa_replace_string(context->datatype, datatype);

   // free the resolved CURIEs
   free(about);
   free(src);
   rdfa_free_list(type_of);
   rdfa_free_list(rel);
   rdfa_free_list(rev);
   free(xml_lang);
   free(content);
   free(resource);
   free(href);
   free(datatype);
}

static void character_data(
      void *parser_context, const xmlChar *s, int len)
{
   //xmlParserCtxtPtr parser = (xmlParserCtxtPtr)parser_context;
   rdfalist* context_stack =
      (rdfalist*)((rdfacontext*)parser_context)->context_stack;
   rdfacontext* context = (rdfacontext*)
      context_stack->items[context_stack->num_items - 1]->data;

   char *buffer = (char*)malloc(len + 1);
   memset(buffer, 0, len + 1);
   memcpy(buffer, s, len);

   // append the text to the current context's plain literal
   if(context->plain_literal == NULL)
   {
      context->plain_literal =
         rdfa_replace_string(context->plain_literal, buffer);
      context->plain_literal_size = len;
   }
   else
   {
      context->plain_literal = rdfa_n_append_string(
         context->plain_literal, &context->plain_literal_size, buffer, len);
   }

   // append the text to the current context's XML literal
   if(context->xml_literal == NULL)
   {
      context->xml_literal =
         rdfa_replace_string(context->xml_literal, buffer);
      context->xml_literal_size = len;
   }
   else
   {
      context->xml_literal = rdfa_n_append_string(
         context->xml_literal, &context->xml_literal_size, buffer, len);
  }

   //printf("plain_literal: %s\n", context->plain_literal);
   //printf("xml_literal: %s\n", context->xml_literal);

   free(buffer);
}

static void end_element(void* parser_context, const char* name,
   const char* prefix,const xmlChar* URI)
{
   //xmlParserCtxtPtr parser = (xmlParserCtxtPtr)parser_context;
   rdfalist* context_stack =
      (rdfalist*)((rdfacontext*)parser_context)->context_stack;
   rdfacontext* context = (rdfacontext*)rdfa_pop_item(context_stack);
   rdfacontext* parent_context = (rdfacontext*)
      context_stack->items[context_stack->num_items - 1]->data;

   // append the text to the current context's XML literal
   char* buffer = (char*)malloc(strlen(name) + 4);

   if(DEBUG)
   {
      printf("DEBUG: </%s>\n", name);
   }

   sprintf(buffer, "</%s>", name);
   if(context->xml_literal == NULL)
   {
      context->xml_literal =
         rdfa_replace_string(context->xml_literal, buffer);
      context->xml_literal_size = strlen(buffer);
   }
   else
   {
      context->xml_literal = rdfa_n_append_string(
         context->xml_literal, &context->xml_literal_size,
         buffer, strlen(buffer));
   }
   free(buffer);

   // 9. The next step of the iteration is to establish any
   // [current object literal];

   // generate the complete object literal triples
   if(context->property != NULL)
   {
      // save the current xml literal
      char* saved_xml_literal = context->xml_literal;
      char* content_start = NULL;
      char* content_end = NULL;

      // ensure to mark only the inner-content of the XML node for
      // processing the object literal.
      buffer = NULL;

      if(context->xml_literal != NULL)
      {
         // get the data between the first tag and the last tag
         content_start = strchr(context->xml_literal, '>');
         content_end = strrchr(context->xml_literal, '<');

         if((content_start != NULL) && (content_end != NULL))
         {
            // set content end to null terminator
            context->xml_literal = ++content_start;
            *content_end = '\0';
         }
      }

      // update the plain literal if the XML Literal is an empty string
      if(strlen(context->xml_literal) == 0)
      {
         context->plain_literal =
            rdfa_replace_string(context->plain_literal, "");
      }

      // process data between first tag and last tag
      // this needs the xml literal to be null terminated
      rdfa_complete_object_literal_triples(context);

      if(content_end != NULL)
      {
         // set content end back
         *content_end = '<';
      }

      if(saved_xml_literal != NULL)
      {
         // restore xml literal
         context->xml_literal = saved_xml_literal;
      }
   }

   //printf(context->plain_literal);

   // append the XML literal and plain text literals to the parent
   // literals
   if(context->xml_literal != NULL)
   {
      if(parent_context->xml_literal == NULL)
      {
         parent_context->xml_literal =
            rdfa_replace_string(
               parent_context->xml_literal, context->xml_literal);
         parent_context->xml_literal_size = context->xml_literal_size;
      }
      else
      {
         parent_context->xml_literal =
            rdfa_n_append_string(
               parent_context->xml_literal, &parent_context->xml_literal_size,
               context->xml_literal, context->xml_literal_size);
      }

      // if there is an XML literal, there is probably a plain literal
      if(context->plain_literal != NULL)
      {
         if(parent_context->plain_literal == NULL)
         {
            parent_context->plain_literal =
               rdfa_replace_string(
                  parent_context->plain_literal, context->plain_literal);
            parent_context->plain_literal_size = context->plain_literal_size;
         }
         else
         {
            parent_context->plain_literal =
               rdfa_n_append_string(
                  parent_context->plain_literal,
                  &parent_context->plain_literal_size,
                  context->plain_literal,
                  context->plain_literal_size);
         }
      }
   }

   // preserve the bnode count by copying it to the parent_context
   parent_context->bnode_count = context->bnode_count;
   parent_context->underscore_colon_bnode_name = \
      rdfa_replace_string(parent_context->underscore_colon_bnode_name,
                          context->underscore_colon_bnode_name);

   // 10. If the [ skip element ] flag is 'false', and [ new subject ]
   // was set to a non-null value, then any [ incomplete triple ]s
   // within the current context should be completed:
   if((context->skip_element == 0) && (context->new_subject != NULL))
   {
      rdfa_complete_incomplete_triples(context);
   }

   // free the context
   rdfa_free_context(context);
}

void rdfa_set_default_graph_triple_handler(
   rdfacontext* context, triple_handler_fp th)
{
   context->default_graph_triple_callback = th;
}

void rdfa_set_processor_graph_triple_handler(
   rdfacontext* context, triple_handler_fp th)
{
   context->processor_graph_triple_callback = th;
}

void rdfa_set_buffer_filler(rdfacontext* context, buffer_filler_fp bf)
{
   context->buffer_filler_callback = bf;
}

#ifndef LIBRDFA_IN_RAPTOR
static void rdfa_report_error(void* parser_context, char* msg, ...)
{
   va_list args;
   va_start(args, msg);
   fprintf(stdout, "libxml2 ERROR: ");
   vfprintf(stdout, msg, args);
   va_end(args);

   //xmlParserCtxtPtr parser = (xmlParserCtxtPtr)parser_context;
   //rdfacontext* context = (rdfacontext*)parser->userData;

   /*
   snprintf(buffer, 2<<12, "XML parsing error: %s at line %d, column %d.",
      XML_ErrorString(XML_GetErrorCode(context->parser)),
      (int)XML_GetCurrentLineNumber(context->parser),
      (int)XML_GetCurrentColumnNumber(context->parser));

   if(context->processor_graph_triple_callback != NULL)
   {
      char* error_subject = rdfa_create_bnode(context);
      char* pointer_subject = rdfa_create_bnode(context);

      // generate the RDFa Processing Graph error type triple
      rdftriple* triple = rdfa_create_triple(
         error_subject, "http://www.w3.org/1999/02/22-rdf-syntax-ns#type",
         "http://www.w3.org/ns/rdfa_processing_graph#Error",
         RDF_TYPE_IRI, NULL, NULL);
      context->processor_graph_triple_callback(triple, context->callback_data);

      // generate the error description
      triple = rdfa_create_triple(
         error_subject, "http://purl.org/dc/terms/description", buffer,
         RDF_TYPE_PLAIN_LITERAL, NULL, "en");
      context->processor_graph_triple_callback(triple, context->callback_data);

      // generate the context triple for the error
      triple = rdfa_create_triple(
         error_subject, "http://www.w3.org/ns/rdfa_processing_graph#context",
         pointer_subject, RDF_TYPE_IRI, NULL, NULL);
      context->processor_graph_triple_callback(triple, context->callback_data);

      // generate the type for the context triple
      triple = rdfa_create_triple(
         pointer_subject, "http://www.w3.org/1999/02/22-rdf-syntax-ns#type",
         "http://www.w3.org/2009/pointers#LineCharPointer",
         RDF_TYPE_IRI, NULL, NULL);
      context->processor_graph_triple_callback(triple, context->callback_data);

      // generate the line number
      snprintf(buffer, 2<<12, "%d",
         (int)XML_GetCurrentLineNumber(context->parser));
      triple = rdfa_create_triple(
         pointer_subject, "http://www.w3.org/2009/pointers#lineNumber",
         buffer, RDF_TYPE_TYPED_LITERAL,
         "http://www.w3.org/2001/XMLSchema#positiveInteger", NULL);
      context->processor_graph_triple_callback(triple, context->callback_data);

      // generate the column number
      snprintf(buffer, 2<<12, "%d",
         (int)XML_GetCurrentColumnNumber(context->parser));
      triple = rdfa_create_triple(
         pointer_subject, "http://www.w3.org/2009/pointers#charNumber",
         buffer, RDF_TYPE_TYPED_LITERAL,
         "http://www.w3.org/2001/XMLSchema#positiveInteger", NULL);
      context->processor_graph_triple_callback(triple, context->callback_data);

      free(error_subject);
      free(pointer_subject);
   }
   else
   {
      printf("librdfa processor error: %s\n", buffer);
   }
   */
}
#endif

int rdfa_parse_start(rdfacontext* context)
{
   // create the buffers and expat parser
   int rval = RDFA_PARSE_SUCCESS;

   context->wb_allocated = sizeof(char) * READ_BUFFER_SIZE;
   // +1 for NUL at end, to allow strstr() etc. to work
   // malloc - only the first char needs to be NUL
   context->working_buffer = (char*)malloc(context->wb_allocated + 1);
   *context->working_buffer = '\0';
   context->done = 0;
   context->context_stack = rdfa_create_list(32);

   // initialize the context stack
   rdfa_push_item(context->context_stack, context, RDFALIST_FLAG_CONTEXT);

#ifdef LIBRDFA_IN_RAPTOR
   context->sax2 = raptor_new_sax2(context->world, context->locator,
                                   context->context_stack);
#else
#endif

   // set up the context stack
#ifdef LIBRDFA_IN_RAPTOR
   raptor_sax2_set_start_element_handler(context->sax2,
                                         raptor_rdfa_start_element);
   raptor_sax2_set_end_element_handler(context->sax2,
                                       raptor_rdfa_end_element);
   raptor_sax2_set_characters_handler(context->sax2,
                                      raptor_rdfa_character_data);
   raptor_sax2_set_namespace_handler(context->sax2,
                                     raptor_rdfa_namespace_handler);
#endif

   rdfa_init_context(context);

#ifdef LIBRDFA_IN_RAPTOR
   context->base_uri=raptor_new_uri(context->sax2->world, (const unsigned char*)context->base);
   raptor_sax2_parse_start(context->sax2, context->base_uri);
#endif

   return rval;
}

int rdfa_parse_chunk(rdfacontext* context, char* data, size_t wblen, int done)
{

   // it is an error to call this before rdfa_parse_start()
   if(context->done)
   {
      return RDFA_PARSE_FAILED;
   }

   if(!context->preread)
   {
      // search for the <base> tag and use the href contained therein to
      // set the parsing context.
      context->wb_preread = rdfa_init_base(context,
         &context->working_buffer, &context->wb_allocated, data, wblen);

      // continue looking if in first 131072 bytes of data
      if(!context->base && context->wb_preread < (1<<17))
         return RDFA_PARSE_SUCCESS;

#ifdef LIBRDFA_IN_RAPTOR

      if(raptor_sax2_parse_chunk(context->sax2,
                                 (const unsigned char*)context->working_buffer,
                                 context->wb_position, done))
      {
         return RDFA_PARSE_FAILED;
      }
#else
      // create the SAX2 handler structure
      xmlSAXHandler handler;
      memset(&handler, 0, sizeof(xmlSAXHandler));
      handler.initialized = XML_SAX2_MAGIC;
      handler.startElementNs = (startElementNsSAX2Func)start_element;
      handler.endElementNs = (endElementNsSAX2Func)end_element;
      handler.characters = (charactersSAXFunc)character_data;
      handler.error = (errorSAXFunc)rdfa_report_error;

      // create a push-based parser
      xmlParserCtxtPtr parser = xmlCreatePushParserCtxt(
         &handler, context, (const char*)context->working_buffer,
         context->wb_position, NULL);

      // ensure that entity substitution is turned on by default
      xmlSubstituteEntitiesDefault(1);

      context->parser = parser;
#endif
      context->preread = 1;

      return RDFA_PARSE_SUCCESS;
   }

   // otherwise just parse the block passed in
#ifdef LIBRDFA_IN_RAPTOR
   if(raptor_sax2_parse_chunk(context->sax2, (const unsigned char*)data, wblen, done))
   {
      return RDFA_PARSE_FAILED;
   }
#else
   if(xmlParseChunk(context->parser, data, wblen, done))
   {
      return RDFA_PARSE_FAILED;
   }
#endif

   return RDFA_PARSE_SUCCESS;
}

void rdfa_parse_end(rdfacontext* context)
{
   // free context stack
   rdfa_free_context_stack(context);

   // Free the expat parser and the like
#ifdef LIBRDFA_IN_RAPTOR
   if(context->base_uri)
      raptor_free_uri(context->base_uri);
   raptor_free_sax2(context->sax2);
   context->sax2=NULL;
#else
   // free parser
   xmlFreeParserCtxt(context->parser);
   xmlCleanupParser();
#endif
}

char* rdfa_get_buffer(rdfacontext* context, size_t* blen)
{
   *blen = context->wb_allocated;
   return context->working_buffer;
}

int rdfa_parse_buffer(rdfacontext* context, size_t bytes)
{
   int rval;
   int done;
   done = (bytes == 0);
   rval = rdfa_parse_chunk(context, context->working_buffer, bytes, done);
   context->done = done;
   return rval;
}

int rdfa_parse(rdfacontext* context)
{
  int rval;

  rval = rdfa_parse_start(context);
  if(rval != RDFA_PARSE_SUCCESS)
  {
    context->done = 1;
    return rval;
  }

  do
  {
     size_t wblen;
     int done;

     wblen = context->buffer_filler_callback(
        context->working_buffer, context->wb_allocated,
        context->callback_data);
     done = (wblen == 0);

     rval = rdfa_parse_chunk(context, context->working_buffer, wblen, done);
     context->done=done;
  }
  while(!context->done && rval == RDFA_PARSE_SUCCESS);

  rdfa_parse_end(context);

  return rval;
}
