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
 *    rdfacontext* context = rdfa_create_context(BASE_URI);
 *    rdfa_set_triple_handler(context, &process_triple);
 *    rdfa_set_buffer_filler(context, &fill_buffer);
 *    rdfa_parse(context);
 *    rdfa_free_context(context);
 *
 * @author Manu Sporny
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <expat.h>
#include "rdfa_utils.h"
#include "rdfa.h"

#define READ_BUFFER_SIZE 4096

// All functions that rdfa.c needs.
void rdfa_update_uri_mappings(
   rdfacontext* context, const char* attribute, const char* value);
void rdfa_update_base(rdfacontext* context, const char* base);
void rdfa_update_language(rdfacontext* context, const char* lang);
void rdfa_establish_new_subject(
   rdfacontext* context, const char* about, const char* src,
   const char* resource, const char* href, rdfalist* instanceof);
void rdfa_establish_new_subject_with_relrev(
   rdfacontext* context, const char* about, const char* src,
   const char* resource, const char* href, rdfalist* instanceof);
void rdfa_complete_incomplete_triples(rdfacontext* context);
void rdfa_complete_type_triples(rdfacontext* context, rdfalist* instanceof);
void rdfa_complete_relrev_triples(
   rdfacontext* context, const rdfalist* rel, const rdfalist* rev);
void rdfa_save_incomplete_triples(
   rdfacontext* context, const rdfalist* rel, const rdfalist* rev);

/**
 * Handles the start_element call
 */
static void XMLCALL
   start_element(void* user_data, const char* name, const char** attributes)
{
   rdfacontext* context = user_data;
   printf("<%s>\n", name);
   
   // 1. First, some of the local values are initialised, as follows:
   // * the [recurse] flag is set to true;
   context->recurse = 0;
   
   // * [new subject] is set to null.
   context->new_subject = NULL;

   // prepare all of the RDFa-specific attributes we are looking for.
   const char** aptr = attributes;
   const char* xml_base = NULL;
   const char* xml_lang = NULL;
   const char* about_curie = NULL;
   char* about = NULL;
   const char* src_curie = NULL;
   char* src = NULL;
   const char* instanceof_curie = NULL;
   rdfalist* instanceof = NULL;
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
   const char* content = NULL;
   const char* datatype = NULL;

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
            about_curie = value;
            about = rdfa_resolve_curie(context, about_curie);
            if((about == NULL) && (about_curie != NULL))
            {
               about = rdfa_resolve_uri(context, about_curie);
            }
         }
         else if(strcmp(attribute, "src") == 0)
         {
            src_curie = value;
            src = rdfa_resolve_curie(context, src_curie);
         }
         else if(strcmp(attribute, "instanceof") == 0)
         {
            instanceof_curie = value;
            instanceof = rdfa_resolve_curie_list(
               context, instanceof_curie, CURIE_PARSE_INSTANCEOF);
         }
         else if(strcmp(attribute, "rel") == 0)
         {
            rel_curie = value;
            rel = rdfa_resolve_curie_list(
               context, rel_curie, CURIE_PARSE_RELREV);
         }
         else if(strcmp(attribute, "rev") == 0)
         {
            rev_curie = value;
            rev = rdfa_resolve_curie_list(
               context, rev_curie, CURIE_PARSE_RELREV);
         }
         else if(strcmp(attribute, "property") == 0)
         {
            property_curie = value;
            property =
               rdfa_resolve_curie_list(
                  context, property_curie, CURIE_PARSE_PROPERTY);
         }
         else if(strcmp(attribute, "resource") == 0)
         {
            resource_curie = value;
            resource = rdfa_resolve_curie(context, resource_curie);
            if((resource == NULL) && (resource_curie != NULL))
            {
               resource = rdfa_resolve_uri(context, resource_curie);
            }
         }
         else if(strcmp(attribute, "href") == 0)
         {
            href_curie = value;
            href = rdfa_resolve_curie(context, href_curie);
         }
         else if(strcmp(attribute, "content") == 0)
         {
            content = value;
         }
         else if(strcmp(attribute, "datatype") == 0)
         {
            datatype = value;
         }
         else if(strcmp(attribute, "xml:lang") == 0)
         {
            xml_lang = value;
         }
         else if(strcmp(name, "base") == 0)
         {
            xml_base = value;
         }
         else if(strstr(attribute, "xmlns") != NULL)
         {
            // 2. Next the [current element] is parsed for [URI
            //    mapping]s and these are added to the [list of URI mappings].
            rdfa_update_uri_mappings(context, attribute, value);
         }
      }
   }
   
   // 2.1 The [current element] is parsed for xml:base and [base] is set
   // to this value if it exists. -- manu (not in the processing rules
   // yet)
   rdfa_update_base(context, xml_base);
   
   // 3. The [current element] is also parsed for any language
   //    information, and [language] is set in the [current
   //    evaluation context];
   rdfa_update_language(context, xml_lang);

   /***************** FOR DEBUGGING PURPOSES ONLY *******************/
   if(about != NULL)
   {
      printf("DEBUG: @about = %s\n", about);
   }
   if(src != NULL)
   {
      printf("DEBUG: @src = %s\n", src);
   }
   if(instanceof != NULL)
   {
      printf("DEBUG: @instanceof = ");
      rdfa_print_list(instanceof);
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
   
   if((rel == NULL) && (rev == NULL))
   {
      // 4. Establish new subject if @rel/@rev don't exist on current
      //    element
      rdfa_establish_new_subject(
         context, about, src, resource, href, instanceof);
   }
   else
   {
      // 5. If the [current element] does contain a valid @rel or
      //    @rev URI, obtained according to the section on CURIE and
      //    URI Processing, then the next step is to establish both a
      //    value for [new subject] and a value for [current object resource]:
      rdfa_establish_new_subject_with_relrev(
         context, about, src, resource, href, instanceof);
   }

   if(context->new_subject != NULL)
   {
      printf("DEBUG: new_subject = %s\n", context->new_subject);
      // 6. If in any of the previous steps a [new subject] was set to a
      //    non-null value, it is now used to:
      
      // complete any incomplete triples;
      rdfa_complete_incomplete_triples(context);

      // provide a subject for type values;
      if(instanceof != NULL)
      {
         rdfa_complete_type_triples(context, instanceof);
      }

      // furnish a new value for [current subject].
      // Once all 'incomplete triples' have been resolved,
      // [current subject] is set to [new subject].
      context->current_subject =
         rdfa_replace_string(context->current_subject, context->new_subject);

      // * The values [parent bnode] and [parent object] are both
      // cleared so that they are not passed on to child statements
      // when recursing.
      if(context->parent_bnode != NULL)
      {
         free(context->parent_bnode);
         context->parent_bnode = NULL;
      }
      if(context->parent_object != NULL)
      {
         free(context->parent_object);
         context->parent_object = NULL;
      }

      // Note that none of this block is executed if there is no
      // [new subject] value, i.e., [new subject] remains null, which
      // means that [current subject] will not be modified, and will
      // remain exactly as it was during the processing of the parent element.
   }

   if(context->current_object_resource != NULL)
   {
      // 7. Process the [current object resource] if it is not null
      rdfa_complete_relrev_triples(context, rel, rev);
   }
   else
   {
      // 8. Save all incomplete triples if no [current object
      //     resource] was found.
      rdfa_save_incomplete_triples(context, rel, rev);
   }

   // 9. The final step of the iteration is to establish any
   //    [current object literal];
   printf("TODO: Implement steps #9 and #10");
   
   // free the resolved CURIEs
   free(about);
   free(src);
   rdfa_free_list(instanceof);
   rdfa_free_list(rel);
   rdfa_free_list(rev);
   rdfa_free_list(property);
   free(resource);
   free(href);
}

static void XMLCALL
   end_element(void *user_data, const char *name)
{
   printf("</%s>", name);
}

rdfacontext* rdfa_create_context(const char* base)
{
   rdfacontext* rval = NULL;
   size_t base_length = strlen(base);

   // if the base isn't specified, don't create a context
   if(base_length > 0)
   {
      rval = malloc(sizeof(rdfacontext));
      rval->base = NULL;
      rval->base = rdfa_replace_string(rval->base, base);
   }
   
   return rval;
}

void rdfa_free_context(rdfacontext* context)
{
   if(context->base)
   {
      free(context->base);
   }
   
   if(context->current_subject != NULL)
   {
      free(context->current_subject);
   }
   
   if(context->parent_object != NULL)
   {
      free(context->parent_object);
   }

   if(context->parent_bnode != NULL)
   {
      free(context->parent_bnode);
   }

   if(context->uri_mappings != NULL)
   {
      rdfa_free_mapping(context->uri_mappings);
   }

   if(context->incomplete_triples != NULL)
   {
      rdfa_free_list(context->incomplete_triples);
   }
   
   if(context->language != NULL)
   {
      free(context->language);
   }

   if(context->new_subject != NULL)
   {
      free(context->new_subject);
   }

   if(context->current_object_resource != NULL)
   {
      free(context->current_object_resource);
   }
      
   free(context);
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
   context->current_subject = NULL;
   if(context->base != NULL)
   {
      context->current_subject =
         rdfa_replace_string(context->current_subject, context->base);
   }

   // the [parent object] is set to null;
   context->parent_object = NULL;
   
   // the [parent bnode] is set to null;
   context->parent_bnode = NULL;
   
   // the [list of URI mappings] is cleared;
   context->uri_mappings = (char**)rdfa_create_mapping(MAX_URI_MAPPINGS);
   
   // the [list of incomplete triples] is cleared;
   context->incomplete_triples = rdfa_create_list(3);
   
   // the [language] is set to null.
   context->language = NULL;

   // set the bnode_count to 0.
   context->bnode_count = 0;

   // set the [current object resource] to null;
   context->current_object_resource = NULL;

   // the next two are initialized to make the C compiler and valgrind
   // happy - they are not a part of the RDFa spec.
   context->recurse = 0;
   context->new_subject = NULL;
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
