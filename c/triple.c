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
   if(triple->object_type == RDF_TYPE_NAMESPACE_PREFIX)
   {
      printf("%s %s: <%s> .\n",
         triple->subject, triple->predicate, triple->object);
   }
   else
   {
      if(triple->subject != NULL)
      {
         if((triple->subject[0] == '_') && (triple->subject[1] == ':'))
         {
            printf("%s\n", triple->subject);
         }
         else
         {
            printf("<%s>\n", triple->subject);
         }
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
            if((triple->object[0] == '_') && (triple->object[1] == ':'))
            {
               printf("      %s", triple->object);
            }
            else
            {
               printf("      <%s>", triple->object);
            }
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
            printf("      \"%s\"^^rdf:XMLLiteral", triple->object);
         }
         else if(triple->object_type == RDF_TYPE_TYPED_LITERAL)
         {
            if((triple->datatype != NULL) && (triple->language != NULL))
            {
               printf("      \"%s\"@%s^^%s",
                  triple->object, triple->language, triple->datatype);
            }
            else if(triple->datatype != NULL)
            {
               printf("      \"%s\"^^%s", triple->object, triple->datatype);
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
 * Generates a namespace prefix triple for any application that is
 * interested in processing namespace changes.
 *
 * @param context the RDFa context.
 * @param prefix the name of the prefix
 * @param IRI the fully qualified IRI that the prefix maps to.
 */
void rdfa_generate_namespace_triple(
   rdfacontext* context, const char* prefix, const char* iri)
{
   rdftriple* triple =
      rdfa_create_triple(
         "@prefix", prefix, iri, RDF_TYPE_NAMESPACE_PREFIX, NULL, NULL);
   context->triple_callback(triple);
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
      
      if(incomplete_triple->flags & RDFALIST_FLAG_FORWARD)
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
      
         rdftriple* triple = rdfa_create_triple(
            context->current_object_resource, curie->data,
            context->current_subject, RDF_TYPE_IRI, NULL, NULL);
      
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
            context->incomplete_triples, curie->data,
               RDFALIST_FLAG_FORWARD | RDFALIST_FLAG_TEXT);
         
         relptr++;
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
            context->incomplete_triples, curie->data,
               RDFALIST_FLAG_REVERSE | RDFALIST_FLAG_TEXT);

         revptr++;
      }
   }   
}

void rdfa_complete_object_literal_triples(rdfacontext* context)
{
   // 9. The final step of the iteration is to establish any
   //    [current object literal];
   //
   // Predicates for the [current object literal] can be set by using
   // @property. If present, a URI is obtained according to the
   // section on CURIE and URI Processing, and then the actual literal
   // value is obtained as follows:
   char* current_object_literal = NULL;
   rdfresource_t type = RDF_TYPE_UNKNOWN;   
   
   // * as a [plain literal] if:
   //   o @content is present;
   //   o or all children of the [current element] are text nodes;
   //   o or there are no child nodes; TODO: Is this needed?
   //   o or the body of the [current element] does have non-text
   //     child nodes but @datatype is present, with an empty value.
   //
   // * Additionally, if there is a value for [current language] then
   // the value of the [plain literal] should include this language
   // information, as described in [RDF-CONCEPTS]. The actual literal
   // is either the value of @content (if present) or a string created
   // by concatenating the text content of each of the child elements
   // of the [current element] in document order, and then normalising
   // white-space according to [WHITESPACERULES].
   //
   // TODO: Whitespace normalization
   if((context->content != NULL))
   {
      current_object_literal = context->content;
      type = RDF_TYPE_PLAIN_LITERAL;
   }
   else if(index(context->xml_literal, '<') == NULL)
   {      
      current_object_literal = context->plain_literal;
      type = RDF_TYPE_PLAIN_LITERAL;
   }
   else if(strlen(context->plain_literal) == 0)
   {
      current_object_literal = "";
      type = RDF_TYPE_PLAIN_LITERAL;
   }
   else if((context->xml_literal != NULL) &&
           (context->datatype != NULL) &&
           (strlen(context->xml_literal) > 0) &&
           (strcmp(context->datatype, "") == 0))
   {
      current_object_literal = context->xml_literal;
      type = RDF_TYPE_PLAIN_LITERAL;
   }

   
   // * as an [XML literal] if:
   //    o the [current element] has any child nodes that are not
   //      simply text nodes, and @datatype is not present, or is
   //      present, but is set to rdf:XMLLiteral.
   //
   // The value of the [XML literal] is a string created from the
   // inner content of the [current element], i.e., not including
   // the element itself, with the datatype of rdf:XMLLiteral.
   if((current_object_literal == NULL) &&
      (index(context->xml_literal, '<') != NULL) &&
      ((context->datatype == NULL) ||
       (strcmp(context->datatype, "rdf:XMLLiteral") == 0)))
   {
      current_object_literal = context->xml_literal;
      type = RDF_TYPE_XML_LITERAL;
   }
   
   // * as a [typed literal] if:
   //    o @datatype is present, and does not have an empty
   //      value.
   //
   // The actual literal is either the value of @content (if
   // present) or a string created by concatenating the inner
   // content of each of the child elements in turn, of the
   // [current element]. The final string includes the datatype
   // URI, as described in [RDF-CONCEPTS], which will have been
   // obtained according to the section on CURIE and URI
   // Processing.      
   if((context->content != NULL) && (context->datatype != NULL) &&
      (strlen(context->datatype) > 0))
   {
      current_object_literal = context->content;
      type = RDF_TYPE_TYPED_LITERAL;
   }

   // TODO: shouldn't this be used with EACH predicate?
   // The [current object literal] is then used with the predicate to
   // generate a triple as follows:
   //
   // subject
   //    [current subject]
   // predicate
   //    full URI
   // object
   //    [current object literal]
   int i;
   rdfalistitem** pptr = context->property->items;
   for(i = 0; i < context->property->num_items; i++)
   {
      
      rdfalistitem* curie = *pptr;
      rdftriple* triple = NULL;
      
      if(type == RDF_TYPE_PLAIN_LITERAL)
      {
         char* canonicalized_literal =
            rdfa_canonicalize_string(current_object_literal);
         triple = rdfa_create_triple(context->current_subject,
            curie->data, canonicalized_literal, type, context->datatype,
            context->language);
         free(canonicalized_literal);
      }
      else
      {
         triple = rdfa_create_triple(context->current_subject,
            curie->data, current_object_literal, type, context->datatype,
            context->language);
      }
      
      context->triple_callback(triple);
      pptr++;
   }

   // TODO: Implement recurse flag being set to false
   //
   // Once the triple has been created, if the [datatype] of the
   // [current object literal] is rdf:XMLLiteral, then the [recurse]
   // flag is set to false

}
