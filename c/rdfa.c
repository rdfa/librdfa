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

// All functions that rdfa.c needs.
char** rdfa_init_mapping(size_t elements);
void rdfa_update_uri_mappings(
   rdfacontext* context, const char* attribute, const char* value);
void rdfa_update_language(rdfacontext* context, const char* xml_lang);

/**
 * Handles the start_element call
 */
static void XMLCALL
   start_element(void* user_data, const char* name, const char** attributes)
{
   rdfacontext* current_context = user_data;
   printf("<%s>\n", name);
   
   //////////////////////////////////
   //First, some of the local values are initialised, as follows:
   // * the [recurse] flag is set to true;
   current_context->recurse = 0;
   
   // * [new subject] is set to null.
   current_context->new_subject = NULL;

   // prepare all of the RDFa-specific attributes we are looking for.
   const char** aptr = attributes;
   const char* xml_lang = NULL;
   const char* about = NULL;
   const char* src = NULL;
   const char* instanceof = NULL;
   const char* rel = NULL;
   const char* rev = NULL;
   const char* property = NULL;
   const char* resource = NULL;
   const char* href = NULL;

   // scan all of the attributes for the RDFa-specific attributes
   if(aptr != NULL)
   {
      while(*aptr != NULL)
      {
         const char* attribute = *aptr;
         aptr++;
         const char* value = *aptr;
         aptr++;

         if(strcmp(attribute, "about") == 0)
         {
            about = value;
         }
         else if(strcmp(attribute, "src") == 0)
         {
            src = value;
         }
         else if(strcmp(attribute, "instanceof") == 0)
         {
            instanceof = value;
         }
         else if(strcmp(attribute, "rel") == 0)
         {
            rel = value;
         }
         else if(strcmp(attribute, "rev") == 0)
         {
            rev = value;
         }
         else if(strcmp(attribute, "property") == 0)
         {
            property = value;
         }
         else if(strcmp(attribute, "resource") == 0)
         {
            resource = value;
         }
         else if(strcmp(attribute, "href") == 0)
         {
            href = value;
         }
         else if(strcmp(attribute, "xml:lang") == 0)
         {
            xml_lang = value;
         }
         else if(strstr(attribute, "xmlns") != NULL)
         {
            // Any changes to the [current evaluation context] are made next:
            // update URI mappings
            rdfa_update_uri_mappings(current_context, attribute, value);
         }
      }
   }

   // set the current language, if it is specified
   rdfa_update_language(current_context, xml_lang);
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
   context->uri_mappings = (char**)rdfa_init_mapping(MAX_URI_MAPPINGS);
   
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
