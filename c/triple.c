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

rdftriple* rdfa_create_triple(const char* subject, const char* predicate,
   const char* object, rdfresource_t object_type, const char* datatype,
   const char* language)
{
   rdftriple* rval = malloc(sizeof(rdftriple));

   // clear the memory
   rval->subject = NULL;
   rval->predicate = NULL;
   rval->object = NULL;
   rval->object_type = object_type;
   rval->datatype = NULL;
   rval->language = NULL;

   // a triple needs a subject, predicate and object at minimum to be
   // considered a triple.
   if((subject != NULL) && (predicate != NULL) && (object != NULL))
   {
      rval->subject = rdfa_replace_string(rval->subject, subject);
      rval->predicate = rdfa_replace_string(rval->predicate, predicate);
      rval->object = rdfa_replace_string(rval->object, object);

      // if the datatype is present, set it
      if(datatype != NULL)
      {
         rval->datatype = rdfa_replace_string(rval->datatype, datatype);
      }

      // if the language was specified, set it
      if(language != NULL)
      {
         rval->language = rdfa_replace_string(rval->language, language);
      }
   }

   return rval;
}

void rdfa_print_triple(rdftriple* triple)
{
   if(triple->subject != NULL)
   {
      printf("<%s>\n", triple->subject);
   }
   else
   {
      printf("INCOMPLETE\n");   
   }

   if(triple->predicate != NULL)
   {
      printf("   <%s>\n", triple->predicate);
   }
   else
   {
      printf("   INCOMPLETE\n");   
   }
   
   if(triple->object != NULL)
   {
      if(triple->object_type == RDF_TYPE_IRI)
      {
         printf("      <%s>", triple->object);
      }
      else if(triple->object_type == RDF_TYPE_PLAIN_LITERAL)
      {
         printf("      \"%s\"", triple->object);
         if(triple->language != NULL)
         {
            printf("@%s", triple->language);
         }
      }
      else if(triple->object_type == RDF_TYPE_XML_LITERAL)
      {
         printf("^^^rdf:XMLLiteral");
      }
      else if(triple->object_type == RDF_TYPE_TYPED_LITERAL)
      {
         if(triple->datatype != NULL)
         {
            printf("^^^%s", triple->datatype);
         }
      }
      else
      {
         printf("      <%s> <---- UNKNOWN OBJECT TYPE", triple->object);
      }

      printf(" .\n");
   }
   else
   {
      printf("      INCOMPLETE .");
   }
}

void rdfa_free_triple(rdftriple* triple)
{
   free(triple->subject);
   free(triple->predicate);
   free(triple->object);
   free(triple->datatype);
   free(triple->language);
}

/**
 * Completes all incomplete triples that are part of the current
 * context by matching the current_subject and the new_subject with
 * the list of incomplete triple predicates.
 *
 * @param context the RDFa context.
 */
void rdfa_complete_incomplete_triples(rdfacontext* context)
{
   // 5.5.6.1 complete any incomplete triples;
   //
   // The [list of incomplete triples] will contain zero or more
   // predicate URIs. If [new subject] is non-null then this list is
   // iterated, and each of the predicates is used with [current
   // subject] and [new subject] to generate a triple. Note that each
   // incomplete triple has a [direction] value that it used to
   // determine what will become the subject, and what the object of
   // each generated triple.

   int i;
   for(i = 0; i < context->incomplete_triples->num_items; i++)
   {
      rdfalist* incomplete_triples = context->incomplete_triples;
      rdfalistitem* incomplete_triple = incomplete_triples->items[i];
      
      if(incomplete_triple->flags == RDFALIST_FLAG_FORWARD)
      {
         // If [direction] is 'forward' then the following triple is generated:
         //
         // subject
         //    [current subject]
         // predicate
         //    the predicate from the iterated incomplete triple
         // object
         //    [new subject]
         rdftriple* triple =
            rdfa_create_triple(context->current_subject,
               incomplete_triple->data, context->new_subject, RDF_TYPE_IRI,
               NULL, NULL);
         context->triple_callback(triple);
      }
      else
      {
         // If [direction] is not 'forward' then this is the triple generated:
         //
         // subject
         //    [new subject]
         // predicate
         //    the predicate from the iterated incomplete triple
         // object
         //    [current subject]
         rdftriple* triple =
            rdfa_create_triple(context->new_subject,
               incomplete_triple->data, context->current_subject, RDF_TYPE_IRI,
               NULL, NULL);
         context->triple_callback(triple);
      }
      free(incomplete_triple);
   }
   context->incomplete_triples->num_items = 0;
}

void rdfa_complete_type_triples(
   rdfacontext* context, const rdfalist* instanceof)
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
   int i;

   rdfalistitem** iptr = instanceof->items;
   for(i = 0; i < instanceof->num_items; i++)
   {
      rdfalistitem* curie = *iptr;
      
      rdftriple* triple = rdfa_create_triple(context->new_subject,
         "http://www.w3.org/1999/02/22-rdf-syntax-ns#type",
         curie->data, RDF_TYPE_IRI, NULL, NULL);
      
      context->triple_callback(triple);
      iptr++;
   }
}

void rdfa_complete_relrev_triples(
   rdfacontext* context, const rdfalist* rel, const rdfalist* rev)
{
   // 7. If in any of the previous steps a [current object resource]
   //    was set to a non-null value, it is now used to generate triples:
   int i;

   // Predicates for the [current object resource] can be set by using
   // one or both of the @rel and @rev attributes.

   // If present, @rel will contain one or more URIs, obtained
   // according to the section on CURIE and URI Processing each of
   // which is used to generate a triple as follows:
   //
   // subject
   //    [current subject]
   // predicate
   //    full URI
   // object
   //    [current object resource]
   if(rel != NULL)
   {
      rdfalistitem** relptr = rel->items;
      for(i = 0; i < rel->num_items; i++)
      {
         rdfalistitem* curie = *relptr;
      
         rdftriple* triple = rdfa_create_triple(context->current_subject,
                                                curie->data, context->current_object_resource, RDF_TYPE_IRI,
                                                NULL, NULL);
      
         context->triple_callback(triple);
         relptr++;
      }
   }

   // If present, @rev will contain one or more URIs, obtained
   // according to the section on CURIE and URI Processing each of which
   // is used to generate a triple as follows:
   //
   // subject
   //    [current object resource]
   // predicate
   //    full URI
   // object
   //    [current subject] 
   if(rev != NULL)
   {
      rdfalistitem** revptr = rev->items;
      for(i = 0; i < rev->num_items; i++)
      {
         rdfalistitem* curie = *revptr;
      
         rdftriple* triple = rdfa_create_triple(context->current_subject,
            curie->data, context->current_object_resource, RDF_TYPE_IRI,
            NULL, NULL);
      
         context->triple_callback(triple);
         revptr++;
      }
   }
}

void rdfa_save_incomplete_triples(
   rdfacontext* context, const rdfalist* rel, const rdfalist* rev)
{
   int i;

   // 8. If however [current object resource] was set to null, but there
   // are predicates present, then they must be stored as 'incomplete triples'
   // pending the discovery of a subject that can be used as the
   // object;
   //
   // Predicates for 'incomplete triples' can be set by using one or
   // both of the @rel and @rev attributes.

   // If present, @rel must contain one or more URIs, obtained
   // according to the section on CURIE and URI Processing each of
   // which is added to the [list of incomplete triples] as follows:
   //
   // predicate
   //    full URI
   // direction
   //    forward
   if(rel != NULL)
   {
      rdfalistitem** relptr = rel->items;
      for(i = 0; i < rel->num_items; i++)
      {
         rdfalistitem* curie = *relptr;

         rdfa_add_item(
            context->incomplete_triples, curie->data, RDFALIST_FLAG_FORWARD);
      }
   }
   
   // If present, @rev must contain one or more URIs, obtained
   // according to the section on CURIE and URI Processing, each of
   // which is added to the [list of incomplete triples] as follows:
   //
   // predicate
   //    full URI
   // direction
   //    reverse
   if(rev != NULL)
   {
      rdfalistitem** revptr = rev->items;
      for(i = 0; i < rev->num_items; i++)
      {
         rdfalistitem* curie = *revptr;

         rdfa_add_item(
            context->incomplete_triples, curie->data, RDFALIST_FLAG_REVERSE);
      }
   }   
}
