/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 *
 * This unit test exercises the CURIE processing functions in librdfa.
 *
 * @author Manu Sporny
 */
#include <stdio.h>
#include <string.h>
#include <rdfa.h>
#include <rdfa_utils.h>

#define XMLNS_DEFAULT_MAPPING "XMLNS_DEFAULT"
// These are all of the @property reserved words in XHTML 1.1 that
// should generate triples.
#define XHTML_PROPERTY_RESERVED_WORDS_SIZE 6
static const char*
   my_g_property_reserved_words[XHTML_PROPERTY_RESERVED_WORDS_SIZE] =
{
   "description", "generator", "keywords", "reference", "robots", "title"
};

// These are all of the @rel/@rev reserved words in XHTML 1.1 that
// should generate triples.
#define XHTML_RELREV_RESERVED_WORDS_SIZE 22
static const char*
   my_g_relrev_reserved_words[XHTML_RELREV_RESERVED_WORDS_SIZE] =
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

// we need this declaration to compile cleanly, we shouldn't be
// calling it directly, but since this is a unit test, that's okay.
void rdfa_init_context(rdfacontext* context);
char* rdfa_resolve_relrev_curie(rdfacontext* context, const char* uri);
char* rdfa_resolve_property_curie(rdfacontext* context, const char* uri);


// typedef for CURIE processing function pointer
typedef char* (*curie_func)(rdfacontext*, const char*, curieparse_t);

// typedef for CURIE list processing function pointer
typedef rdfalist* (*curie_list_func)(rdfacontext*, const char*, curieparse_t);

// the number of tests run
int g_test_num = 0;

// the number of tests that passed
int g_test_passes = 0;

// the number of tests that failed
int g_test_fails = 0;

/**
 * Runs a single unit test given the RDFa context, name of the test,
 * test CURIE, processing function and what the final IRI should be.
 *
 * @param context the RDFa context.
 * @param name the name of the test.
 * @param curie the CURIE to resolve.
 * @param cb the function callback to the CURIE resolution function.
 * @param iri the value of what the resulting IRI should be.
 */
void run_test(rdfacontext* context, const char* name, const char* curie,
   curie_func cb, const char* iri, curieparse_t mode)
{
   char* result = cb(context, curie, mode);
   int compare = -1;

   // check to see if we should check for NULL or if the strings
   // should match.
   if(iri != NULL)
   {
      compare = strcmp(result, iri);
   }
   else if(iri == result)
   {
      compare = 0;
   }

   printf("UT#%02i/%s \"%s\" ...", ++g_test_num, name, curie);

   // if the string compare shows identical values, pass the test,
   // otherwise, fail the test.
   if(compare == 0)
   {
      printf("PASS.\n");
      g_test_passes++;
   }
   else
   {
      printf("FAIL. Got \"%s\", but should have been \"%s\".\n", result, iri);
      g_test_fails++;
   }

   if(result != NULL)
   {
      free(result);
   }
}

/**
 * Runs a single unit test given the RDFa context, name of the test,
 * a list of test CURIEs, processing function, and a list of output IRIs.
 *
 * @param context the RDFa context.
 * @param name the name of the test.
 * @param curie the CURIE to resolve.
 * @param cb the function callback to the CURIE resolution function.
 * @param iris a list of output IRIs.
 */
void run_list_test(rdfacontext* context, const char* name, const char* curies,
   curie_list_func cb, rdfalist* iris, curieparse_t mode)
{
   rdfalist* result = cb(context, curies, mode);
   int compare = -1;

   if(result != NULL)
   {
      compare = -1;
      int i;
      rdfalistitem** iptr = iris->items;
      rdfalistitem** rptr = result->items;
      for(i = 0; i < result->num_items; i++)
      {
         compare = 0;
         char* icurie = (*iptr)->data;
         char* rcurie = (*rptr)->data;
         
         //printf("curie list: %s == %s\n", icurie, rcurie);
         if(strcmp(icurie, rcurie) != 0)
         {
            compare = -1;
         }
         iptr++;
         rptr++;
      }
   }   

   printf("UT#%02i/%s \"%s\" ...", ++g_test_num, name, curies);

   // if the string compare shows identical values, pass the test,
   // otherwise, fail the test.
   if(compare == 0)
   {
      printf("PASS.\n");
      g_test_passes++;
   }
   else
   {
      printf("FAIL.");
      
      g_test_fails++;
   }

   if(result != NULL)
   {
      rdfa_free_list(result);
   }
}

/**
 * Runs a set of unit tests given the RDFa context, base name of the test,
 * a set of CURIEs, a processing function, and a base IRI.
 *
 * @param context the RDFa context.
 * @param name the base name of the test.
 * @param curie the set of CURIEs to resolve.
 * @param cb the function callback to the CURIE resolution function.
 * @param iri the base value of what the resulting IRI should be, the
 *            value of each set member will be appended to the IRI.
 */
void run_test_set(rdfacontext* context, const char* name, const char** curies,
   size_t curies_size, curie_func cb, const char* iri, curieparse_t mode)
{
   int i;
   for(i = 0; i < curies_size; i++)
   {
      char* full_iri = rdfa_join_string(iri, curies[i]);
      
      run_test(context, name, curies[i], cb, full_iri, mode);

      free(full_iri);
   }
}

void run_curie_tests()
{
   rdfacontext* context =
      rdfa_create_context("http://example.org/");
   rdfa_init_context(context);
   
   rdfa_update_mapping(
      context->uri_mappings, "dc", "http://purl.org/dc/elements/1.1/");
   rdfa_update_mapping(
      context->uri_mappings, "dctv", "http://purl.org/dc/dcmitype/");
   
   printf("------------------------ CURIE tests ---------------------\n");
   
   run_test(context, "IRI", "http://www.example.org/iri",
            rdfa_resolve_curie, "http://www.example.org/iri",
            CURIE_PARSE_HREF_SRC);
   run_test(context, "Safe CURIE", "[dc:title]",
            rdfa_resolve_curie, "http://purl.org/dc/elements/1.1/title",
            CURIE_PARSE_PROPERTY);
   run_test(context, "Unsafe CURIE", "dc:title",
            rdfa_resolve_curie, "http://purl.org/dc/elements/1.1/title",
            CURIE_PARSE_PROPERTY);
   run_test(context, "Non-prefixed CURIE", ":nonprefixed",
            rdfa_resolve_curie, "http://example.org/nonprefixed",
            CURIE_PARSE_PROPERTY);
   run_test(context, "Reference-only CURIE", "foobar",
            rdfa_resolve_curie, NULL, CURIE_PARSE_PROPERTY);
   run_test(context, "Reference-only safe CURIE", "[foobar]",
            rdfa_resolve_curie, NULL, CURIE_PARSE_PROPERTY);
   run_test(context, "Empty safe CURIE", "[]",
            rdfa_resolve_curie, NULL, CURIE_PARSE_PROPERTY);
   run_test(context, "Blank named safe CURIE", "[_:frank]",
            rdfa_resolve_curie, "_:frank", CURIE_PARSE_PROPERTY);

   rdfalist* dctvlist = rdfa_create_list(2);
   rdfa_add_item(
      dctvlist, "http://purl.org/dc/dcmitype/Image", RDFALIST_FLAG_NONE);
   rdfa_add_item(
      dctvlist, "http://purl.org/dc/dcmitype/Sound", RDFALIST_FLAG_NONE);
   run_list_test(
      context, "XHTML multiple @instanceof", "[dctv:Image] [dctv:Sound]",
      rdfa_resolve_curie_list, dctvlist, CURIE_PARSE_INSTANCEOF_DATATYPE);
   rdfa_free_list(dctvlist);

   rdfalist* nllist = rdfa_create_list(2);
   rdfa_add_item(
      nllist, XHTML_VOCAB_URI "next", RDFALIST_FLAG_NONE);
   rdfa_add_item(
      nllist, XHTML_VOCAB_URI "license", RDFALIST_FLAG_NONE);
   run_list_test(
      context, "XHTML multiple @rel/@rev", "next license",
      rdfa_resolve_curie_list, nllist, CURIE_PARSE_RELREV);
   rdfa_free_list(nllist);
   
   rdfalist* dtlist = rdfa_create_list(2);
   rdfa_add_item(
      dtlist, XHTML_VOCAB_URI "description", RDFALIST_FLAG_NONE);
   rdfa_add_item(
      dtlist, XHTML_VOCAB_URI "title", RDFALIST_FLAG_NONE);   
   run_list_test(
      context, "XHTML multiple @property", "description title",
      rdfa_resolve_curie_list, dtlist, CURIE_PARSE_PROPERTY);
   rdfa_free_list(dtlist);

   run_test_set(context, "XHTML @rel/@rev reserved",
      my_g_relrev_reserved_words, XHTML_RELREV_RESERVED_WORDS_SIZE,
      rdfa_resolve_relrev_curie, XHTML_VOCAB_URI, CURIE_PARSE_RELREV);
   run_test_set(context, "XHTML @property reserved",
      my_g_property_reserved_words, XHTML_PROPERTY_RESERVED_WORDS_SIZE,
      rdfa_resolve_property_curie, XHTML_VOCAB_URI, CURIE_PARSE_PROPERTY);

   printf("---------------------- CURIE test results ---------------------\n"
          "%i passed, %i failed\n",
          g_test_passes, g_test_fails);

   rdfa_free_context(context);
}

int main(int argc, char** argv)
{
   printf("Running CURIE tests\n");
   run_curie_tests();
   
   return 0;
}
