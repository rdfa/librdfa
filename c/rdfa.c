/**
 * The librdfa library is the Fastest RDFa Parser in the Universe. It is
 * a stream parser, meaning that it takes an XML data as input and spits
 * out RDF triples as it comes across them in the stream. Due to this
 * processing approach, librdfa has a very, very small memory footprint.
 * It is also very fast and can operate on hundreds of gigabytes of XML
 * data without breaking a sweat.
 *
 * Usage:
 *
 * rdfacontext* context = rdfa_create_context(base_uri);
 * rdfa_set_triple_handler(context, func);
 * rdfa_set_buffer_filler(context, func);
 * rdfa_parse(context);
 * rdfa_destroy_parser(context);
 *
 * @author Manu Sporny
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <expat.h>
#include "rdfa.h"

#define READ_BUFFER_SIZE 4096

// All functions that rdfa.c makes use of.
void rdfa_update_uri_mappings(rdfacontext* context, const char** attributes);

/**
 * Handles the start_element call
 */
static void XMLCALL
   start_element(void* user_data, const char* name, const char** attributes)
{
   rdfacontext* current_context = user_data;
   printf("<%s>", name);
   
   //////////////////////////////////
   //First, some of the local values are initialised, as follows:
   // * the [recurse] flag is set to true;
   int recurse = 0;
   
   // * [new subject] is set to null.
   char* new_subject = NULL;

   // Any changes to the [current evaluation context] are made next:
   // update URI mappings
   rdfa_update_uri_mappings(current_context, attributes);
}

static void XMLCALL
   end_element(void *user_data, const char *name)
{
}

rdfacontext* rdfa_create_context(const char* base)
{
   rdfacontext* rval = NULL;
   size_t base_length = strlen(base);

   // if the base isn't specified, don't create a context
   if(base_length > 0)
   {
      rval = malloc(sizeof(rdfacontext));
      char* base_copy = malloc(base_length);
      strcpy(base_copy, base);
      rval->base = base_copy;
   }
   
   return rval;
}

void rdfa_set_triple_handler(rdfacontext* context, triple_handler_fp th)
{
   context->triple_callback = th;
}

void rdfa_set_buffer_filler(rdfacontext* context, buffer_filler_fp bf)
{
   context->buffer_filler_callback = bf;
}

void rdfa_init_context(rdfacontext* context)
{
   // the [current subject] is set to the [base] value;
   size_t base_length = strlen(context->base);
   if(base_length > 0)
   {
      context->current_subject = malloc(base_length);
      strcpy(context->current_subject, context->base);
   }

   // the [parent object] is set to null;
   context->parent_object = NULL;
   
   // the [parent bnode] is set to null;
   context->parent_bnode = NULL;
   
   // the [list of URI mappings] is cleared;
   context->uri_mappings = rdfa_init_mapping(MAX_URI_MAPPINGS);
   
   // the [list of incomplete triples] is cleared;
   context->incomplete_triples = NULL;
   
   // the [language] is set to null.
   context->language = NULL;
}

int rdfa_parse(rdfacontext* context)
{
   char buf[READ_BUFFER_SIZE];
   XML_Parser parser = XML_ParserCreate(NULL);
   int done;
   
   XML_SetUserData(parser, context);
   XML_SetElementHandler(parser, start_element, end_element);

   rdfa_init_context(context);
   
   do
   {
      size_t len = context->buffer_filler_callback(buf, sizeof(buf));
      done = len < sizeof(buf);
      if(XML_Parse(parser, buf, len, done) == XML_STATUS_ERROR)
      {
         fprintf(stderr,
                 "%s at line %d\n",
                 XML_ErrorString(XML_GetErrorCode(parser)),
                 XML_GetCurrentLineNumber(parser));
         return 1;
      }
   }
   while(!done);

   XML_ParserFree(parser);
   
   return RDFA_PARSE_FAILED;
}

void rdfa_destroy_context(rdfacontext* context)
{
   if(context->base != NULL)
   {
      free(context->base);
   }
   
   free(context);
}
