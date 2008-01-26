/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include <stdio.h>
#include <rdfa_utils.h>
#include <rdfa.h>

#define XMLNS_DEFAULT_MAPPING "XMLNS_DEFAULT"

// we need this declaration to compile cleanly, we shouldn't be
// calling it directly, but since this is a unit test, that's okay.
void rdfa_init_context(rdfacontext* context);

void run_curie_tests()
{
   rdfacontext* context =
      rdfa_create_context("http://example.org/example.html");
   rdfa_init_context(context);
   
   rdfa_update_mapping(
      context->uri_mappings, "dc", "http://purl.org/dc/elements/1.1/");
      
   printf("Safe CURIE [dc:title]: %s\n",
      rdfa_resolve_curie(context, "[dc:title]"));
   printf("Unsafe CURIE dc:title: %s\n",
      rdfa_resolve_curie(context, "dc:title"));
   printf("Non-prefixed CURIE :unprefixed: %s\n",
      rdfa_resolve_curie(context, ":unprefixed"));
   printf("XHTML reserved word next: %s\n",
      rdfa_resolve_legacy_curie(context, "next"));
   printf("XHTML, non-reserved [foobar]: %s\n",
      rdfa_resolve_curie(context, "[foobar]"));
   printf("XHTML, non-reserved [foobar]: %s\n",
      rdfa_resolve_legacy_curie(context, "[foobar]"));
}

int main(int argc, char** argv)
{
   printf("Running CURIE tests\n");
   run_curie_tests();
   
   return 0;
}
