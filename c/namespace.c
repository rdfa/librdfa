/**
 * Copyright 2008 Digital Bazaar, Inc.
 * This file is a part of librdfa and is licensed under the GNU LGPL v3.
 *
 * This file implements mapping data structure memory management as
 * well as updating URI mappings.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rdfa_utils.h"
#include "rdfa.h"

#define XMLNS_DEFAULT_MAPPING "XMLNS_DEFAULT"

// pre-define functions that we will need in this module
void rdfa_generate_namespace_triple(
   rdfacontext* context, const char* prefix, const char* iri);
   
/**
 * Attempts to update the uri mappings in the given context using the
 * given attribute/value pair.
 *
 * @param attribute the attribute, which must start with xmlns.
 * @param value the value of the attribute
 */
void rdfa_update_uri_mappings(
   rdfacontext* context, const char* attribute, const char* value)
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
   
   if(strcmp(attribute, "xmlns") == 0)
   {
      rdfa_update_mapping(
         context->uri_mappings, XMLNS_DEFAULT_MAPPING, value);
   }
   else if(strstr(attribute, "xmlns:") != NULL)
   {
      // check to make sure we're actually dealing with an
      // xmlns: namespace attribute
      if((attribute[5] == ':') && (attribute[6] != '\0'))
      {
         rdfa_generate_namespace_triple(context, &attribute[6], value);
         rdfa_update_mapping(
            context->uri_mappings, &attribute[6], value);
      }
   }
}

/**
 * Updates the base value for the given context given the new value
 * for the base value.
 *
 * @param context the context that should be modified
 * @param base the new value for the base value.
 */
void rdfa_update_base(rdfacontext* context, const char* base)
{
   if(base != NULL)
   {
      rdfa_generate_namespace_triple(context, "base", base);
      rdfa_replace_string(context->base, base);
   }
}
