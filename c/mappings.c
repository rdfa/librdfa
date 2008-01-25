/**
 * This file implements mapping data structure memory management as
 * well as updating URI mappings.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rdfa.h"

#define XMLNS_DEFAULT_MAPPING "XMLNS_DEFAULT"

/**
 * Initializes a mapping given the number of elements the mapping is
 * expected to hold.
 *
 * @param elements the maximum number of elements the mapping is
 *                 supposed to hold.
 *
 * @return an initialized char**, with all of the elements set to NULL.
 */
char** rdfa_init_mapping(size_t elements)
{
   char** mapping = malloc(sizeof(char*) * elements * 2);
   int i;
   char** mptr = mapping;

   if(mapping != NULL)
   {
      for(i = 0; i < MAX_URI_MAPPINGS * 2; i++)
      {
         *mptr++ = NULL;
      }
   }
   
   return mapping;
}

/**
 * Updates the given mapping when presented with a key and a value. If
 * the key doesn't exist in the mapping, it is created.
 *
 * @param mapping the mapping to update.
 * @param key the key.
 * @param value the value.
 */
void rdfa_update_mapping(char** mapping, const char* key, const char* value)
{
   int found = 0;
   char** mptr = mapping;
   size_t value_length = strlen(value);
   
   // search the current mapping to see if the namespace
   // prefix exists in the mapping
   while(*mptr != NULL)
   {
      if(strcmp(*mptr, key) == 0)
      {
         mptr++;
         free(*mptr);
         *mptr = malloc(value_length);
         strcpy(*mptr, value);
         found = 1;
      }
      else
      {
         mptr++;
         mptr++;
      }
   }

   // if we made it through the entire URI mapping and the key was not
   // found, create a new key-value pair.
   if(!found)
   {
      size_t key_length = strlen(key);
      
      *mptr = malloc(key_length);
      strcpy(*mptr, key);
      
      mptr++;
      *mptr = malloc(value_length);
      strcpy(*mptr, value);
   }

   /* This is only in here for debugging purposes */
   mptr = mapping;
   printf("uri mappings:\n");
   while(*mptr != NULL)
   {
      char* key = *mptr;
      mptr++;
      char* value = *mptr;
      mptr++;

      printf("%20s: %s\n", key, value);
   }
}

/**
 * Attempts to update the uri mappings in the given context using the
 * given attribute/value pair.
 *
 * @param attribute the attribute, which must start with xmlns.
 * @param value the value of the attribute
 */
void rdfa_update_uri_mappings(
   rdfacontext* context, const char* attribute, const char* value)
{
   // * the [current element] is parsed for [URI mappings] and these
   // are added to the [list of URI mappings]. Note that a [URI
   // mapping] will simply overwrite any current mapping in the list
   // that has the same name;
   
   // Mappings are provided by @xmlns. The value to be mapped is set
   // by the XML namespace prefix, and the value to map is the value
   // of the attribute -- a URI. Note that the URI is not processed
   // in any way; in particular if it is a relative path it is not
   // resolved against the [current base]. Authors are advised to
   // follow best practice for using namespaces, which includes not
   // using relative paths.
   
   printf("Attribute: %s = %s\n", attribute, value);
   if(strcmp(attribute, "xmlns") == 0)
   {
      rdfa_update_mapping(
         context->uri_mappings, XMLNS_DEFAULT_MAPPING, value);
   }
   else if(strstr(attribute, "xmlns:") != NULL)
   {
      // check to make sure we're actually dealing with an
      // xmlns: namespace attribute
      if((attribute[5] == ':') && (attribute[6] != '\0'))
      {
         rdfa_update_mapping(
            context->uri_mappings, &attribute[6], value);
      }
   }
}
