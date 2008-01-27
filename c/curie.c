#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "rdfa_utils.h"
#include "rdfa.h"

// These are all of the @property reserved words in XHTML 1.1 that
// should generate triples.
const char* g_property_reserved_words[6] =
{
   "description",
   "generator",
   "keywords",
   "reference",
   "robots",
   "title"
};
   

// These are all of the @rel/@rev reserved words in XHTML 1.1 that
// should generate triples.
const char* g_relrev_reserved_words[22] =
{
   "alternate",
   "appendix",
   "bookmark",
   "chapter",
   "cite",
   "contents",
   "copyright",
   "glossary",
   "help",
   "icon",
   "index",
   "meta",
   "next",
   "p3pv1",
   "prev",
   "role",
   "section",
   "subsection",
   "start",
   "license",
   "up",
   "last"
};

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
         // an IRI contains at least a "://", such as "file://" or "http://"
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
         // if none of the above match, then the CURIE is a reference
         // of some sort.
         rval = CURIE_TYPE_REFERENCE;
      }      
   }

   return rval;
}

/**
 * Expands the given prefix if it is defined in the given context's
 * URI mappings.
 *
 * @param context the context to use when resolving the prefix.
 * @param prefix the shortened prefix to attempt resolving.
 */
char* rdfa_expand_prefix(rdfacontext* context, const char* prefix)
{
   // check to see if the prefix exists in a mapping
   return NULL;
}

char* rdfa_resolve_curie(rdfacontext* context, const char* uri)
{
   char* rval = NULL;
   curie_t ctype = get_curie_type(uri);

   if(ctype == CURIE_TYPE_INVALID)
   {
      rval = NULL;
   }
   else if(ctype == CURIE_TYPE_IRI)
   {
      // if the CURIE is a complete IRI, then return the IRI verbatim
      rval = rdfa_replace_string(rval, uri);
   }
   else if((ctype == CURIE_TYPE_SAFE) || (ctype == CURIE_TYPE_UNSAFE) ||
      (ctype == CURIE_TYPE_REFERENCE))
   {
      char* working_copy = NULL;
      working_copy = rdfa_replace_string(working_copy, uri);
      char* prefix_start = working_copy;
      char* reference_start = working_copy;
      char* reference_end = prefix_start + strlen(working_copy);
      char* colon = strchr(working_copy, ':');

      // if this is a safe CURIE, chop off the beginning and the end
      if(ctype == CURIE_TYPE_SAFE)
      {
         *prefix_start = (int)NULL;
         *reference_end = (int)NULL;
         prefix_start++;
         reference_end--;
      }

      // delimit the colon as null because we'll be using the prefix
      // and the reference in-line (both must be null-terminated)
      if(colon != NULL)
      {
         *colon = (int)NULL;
         reference_start = ++colon;
      }
      else
      {
         reference_start = prefix_start;
      }

      // fully resolve the prefix and get it's length
      const char* expanded_prefix = NULL;
      size_t expanded_prefix_length = 0;
      if(prefix_start != NULL);
      {
         expanded_prefix =
            rdfa_get_mapping(context->uri_mappings, prefix_start);
         if(expanded_prefix != NULL)
         {
            expanded_prefix_length = strlen(expanded_prefix);
         }
      }

      // if the expanded prefix and the reference exist, generate the
      // full IRI.
      if((expanded_prefix != NULL) && (reference_start != NULL))
      {
         size_t reference_size = strlen(reference_start);
         size_t total_size = expanded_prefix_length + reference_size;
         rval = malloc(total_size);
         
         strcpy(rval, expanded_prefix);
         memcpy(
            rval + expanded_prefix_length, reference_start, reference_size);
      }
      else
      {
         // even though a reference-only CURIE is valid, it does not
         // generate a triple in XHTML+RDFa.
         rval = NULL;
      }
   }
   
   return rval;
}

char* rdfa_resolve_legacy_curie(rdfacontext* context, const char* uri)
{

   
   return NULL;
}
