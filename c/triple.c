/**
 * Handles all triple functionality including all incomplete triple
 * functionality.
 *
 * @author Manu Sporny
 */
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "rdfa_utils.h"
#include "rdfa.h"

/**
 * Completes all incomplete triples that are part of the current
 * context by matching the current_subject and the new_subject with
 * the list of incomplete triple predicates.
 *
 * @param context the RDFa context.
 */
void rdfa_complete_incomplete_triples(rdfacontext* context)
{
   
}

void rdfa_complete_type_triples(rdfacontext* context, const char* instanceof)
{
   // 6.2 One or more 'types' for the [new subject] can be set by using
   //     @instanceof. If present, the attribute must contain one or more
   //     URIs, obtained according to the section on URI and CURIE
   //     Processing, each of which is used to generate a triple as follows:
   //
   //     subject
   //        [new subject]
   //     predicate
   //        http://www.w3.org/1999/02/22-rdf-syntax-ns#type
   //     object
   //        full URI of 'type'
   char* working_instanceof = NULL;
   char* iptr = NULL;
   char* ctoken = NULL;
   working_instanceof = rdfa_replace_string(working_instanceof, instanceof);

   ctoken = strtok_r(working_instanceof, " ", &iptr);
   while(ctoken != NULL)
   {
      rdftriple* triple = rfda_create_triple(context->new_subject,
         "http://www.w3.org/1999/02/22-rdf-syntax-ns#type", ctoken, NULL,
         NULL);
      
      context->triple_callback(triple);
      ctoken = strtok_r(NULL, " ", &iptr);
   }
   
   free(working_instanceof);
}
