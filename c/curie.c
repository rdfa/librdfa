#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "rdfa_utils.h"
#include "rdfa.h"

/**
 * Gets the type of CURIE that is passed to it.
 *
 * @param uri the uri to check.
 *
 * @return either CURIE_TYPE_SAFE, CURIE_TYPE_URI or CURIE_TYPE_INVALID.
 */
curie_t get_curie_type(const char* uri)
{
   curie_t rval = CURIE_TYPE_INVALID;
   
   if(uri != NULL)
   {
      size_t uri_length = strlen(uri);

      if((uri[0] == '[') && (uri[uri_length] == ']'))
      {
         // a safe curie starts with [ and ends with ]
         rval = CURIE_TYPE_SAFE;
      }
      else if(strstr(uri, ":/") != NULL)
      {
         // an IRI contains at least a ":/", such as "file:/" or "http:/"
         rval = CURIE_TYPE_IRI;
      }
      else if(strchr(uri, ':') != NULL)
      {
         // an unsafe curie isn't surrounded by brackets, but contains
         // a ':'
         rval = CURIE_TYPE_UNSAFE;
      }
      else
      {
         rval = CURIE_TYPE_REFERENCE;
      }      
   }

   return rval;
}

char* rdfa_resolve_curie(rdfacontext* context, const char* uri)
{
   char* rval = NULL;
   
   return NULL;
}

char* rdfa_resolve_legacy_curie(rdfacontext* context, const char* uri)
{
   return NULL;
}
