/**
 * The CURIE module is used to resolve all forms of CURIEs that
 * XHTML+RDFa accepts.
 *
 * @author Manu Sporny
 */
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "rdfa_utils.h"
#include "rdfa.h"

// These are all of the @property reserved words in XHTML 1.1 that
// should generate triples.
#define XHTML_PROPERTY_RESERVED_WORDS_SIZE 6
static
   const char* g_property_reserved_words[XHTML_PROPERTY_RESERVED_WORDS_SIZE] =
{
   "description", "generator", "keywords", "reference", "robots", "title"
};

// These are all of the @rel/@rev reserved words in XHTML 1.1 that
// should generate triples.
#define XHTML_RELREV_RESERVED_WORDS_SIZE 22
static const char* g_relrev_reserved_words[XHTML_RELREV_RESERVED_WORDS_SIZE] =
{
   "alternate", "appendix", "bookmark", "chapter", "cite", "contents",
   "copyright", "glossary", "help", "icon", "index", "meta", "next", "p3pv1",
   "prev", "role",  "section",  "subsection",  "start", "license", "up", "last"
};

// The base XHTML vocab URL is used to resolve URIs that are reserved
// words. Any reserved listed above is appended to the URL below to
// form a complete IRI.
#define XHTML_VOCAB_URI "http://www.w3.org/1999/xhtml/vocab#"
#define XHTML_VOCAB_URI_SIZE 35

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

      if((uri[0] == '[') && (uri[uri_length - 1] == ']'))
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
   else if((ctype == CURIE_TYPE_SAFE) || (ctype == CURIE_TYPE_UNSAFE))
   {
      char* working_copy = NULL;
      working_copy = rdfa_replace_string(working_copy, uri);
      char* prefix_start = working_copy;
      char* reference_start = working_copy;
      char* reference_end = prefix_start + strlen(working_copy) - 1;
      char* colon = strchr(working_copy, ':');

      // if this is a safe CURIE, chop off the beginning and the end
      if(ctype == CURIE_TYPE_SAFE)
      {
         *prefix_start = '\0';
         *reference_end = '\0';
         prefix_start++;
         reference_end--;
      }

      // delimit the colon as null because we'll be using the prefix
      // and the reference in-line (both must be null-terminated)
      if(colon != NULL)
      {
         *colon = (int)NULL;
         reference_start = colon + 1;
      }
      else
      {
         reference_start = prefix_start;
      }

      // fully resolve the prefix and get it's length
      const char* expanded_prefix = NULL;
      size_t expanded_prefix_length = 0;

      // if a colon was found, but no prefix, use the context->base as
      // the prefix IRI
      if((colon != NULL) && (colon == prefix_start))
      {
         expanded_prefix = context->base;
      }
      else if(prefix_start != NULL)
      {
         // if the prefix was defined, get it from the set of URI mappings.
         expanded_prefix =
            rdfa_get_mapping(context->uri_mappings, prefix_start);
      }

      // get the length of the expanded prefix if it exists.
      if(expanded_prefix != NULL)
      {
         expanded_prefix_length = strlen(expanded_prefix);
      }
      
      // if the expanded prefix and the reference exist, generate the
      // full IRI.
      if((expanded_prefix != NULL) && (*reference_start != '\0'))
      {
         rval = rdfa_join_string(expanded_prefix, reference_start);
      }
   }
   else
   {
      // even though a reference-only CURIE is valid, it does not
      // generate a triple in XHTML+RDFa.
      rval = NULL;
   }
   
   return rval;
}

char* rdfa_resolve_relrev_curie(rdfacontext* context, const char* uri)
{
   char* rval = NULL;
   int i = 0;

   // search all of the XHTML @rel/@rev reserved words for a match
   // against the given URI
   for(i = 0; i < XHTML_RELREV_RESERVED_WORDS_SIZE; i++)
   {
      if(strcmp(g_relrev_reserved_words[i], uri) == 0)
      {
         // since the URI is a reserved word for @rel/@rev, generate
         // the full IRI and stop the loop.
         rval = rdfa_join_string(XHTML_VOCAB_URI, uri);         
         i = XHTML_RELREV_RESERVED_WORDS_SIZE;
      }
   }

   // if none of the XHTML @rel/@rev reserved words were found,
   // attempt to resolve the value as a standard CURIE
   if(rval == NULL)
   {
      rval = rdfa_resolve_curie(context, uri);
   }
   
   return rval;
}

char* rdfa_resolve_property_curie(rdfacontext* context, const char* uri)
{
   char* rval = NULL;
   int i = 0;

   // search all of the XHTML @property reserved words for a match
   // against the given URI
   for(i = 0; i < XHTML_PROPERTY_RESERVED_WORDS_SIZE; i++)
   {
      if(strcmp(g_property_reserved_words[i], uri) == 0)
      {
         // since the URI is a reserved word, generate the full IRI
         // and stop the loop.
         rval = rdfa_join_string(XHTML_VOCAB_URI, uri);
         i = XHTML_PROPERTY_RESERVED_WORDS_SIZE;
      }
   }

   // if none of the XHTML @property reserved words were found,
   // attempt to resolve the value as a standard CURIE
   if(rval == NULL)
   {
      rval = rdfa_resolve_curie(context, uri);
   }
   
   return rval;
}
