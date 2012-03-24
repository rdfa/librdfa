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
 * This file implements mapping data structure memory management as
 * well as updating URI mappings.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rdfa_utils.h"
#include "rdfa.h"

#ifndef LIBRDFA_IN_RAPTOR
/**
 * Attempts to update the uri mappings in the given context using the
 * given attribute/value pair.
 *
 * @param attribute the attribute, which must start with xmlns.
 * @param value the value of the attribute
 */
void rdfa_update_uri_mappings(
   rdfacontext* context, const char* attr, const char* value)
{
   // * the [current element] is parsed for [URI mappings] and these
   // are added to the [list of URI mappings]. Note that a [URI
   // mapping] will simply overwrite any current mapping in the list
   // that has the same name;

   // Mappings are provided by @xmlns. The value to be mapped is set
   // by the XML namespace prefix, and the value to map is the value
   // of the attribute -- a URI. Note that the URI is not processed
   // in any way; in particular if it is a relative path it is not
   // resolved against the [current base]. Authors are advised to
   // follow best practice for using namespaces, which includes not
   // using relative paths.

   if(attr == NULL)
   {
      rdfa_update_mapping(
         context->uri_mappings, XMLNS_DEFAULT_MAPPING, value,
         (update_mapping_value_fp)rdfa_replace_string);
   }
   else if(strcmp(attr, "_") == 0)
   {
      rdfa_processor_triples(context,
         RDFA_PROCESSOR_WARNING,
         "The underscore character must not be declared as a prefix "
         "because it conflicts with the prefix for blank node identifiers. "
         "The occurrence of this prefix declaration is being ignored.");
   }
   else
   {
      rdfa_generate_namespace_triple(context, attr, value);
      rdfa_update_mapping(context->uri_mappings, attr, value,
         (update_mapping_value_fp)rdfa_replace_string);
   }

   // print the current mapping
   if(DEBUG)
   {
      printf("DEBUG: PREFIX MAPPINGS:");
      rdfa_print_mapping(context->uri_mappings,
         (print_mapping_value_fp)rdfa_print_string);
   }
}
#endif
