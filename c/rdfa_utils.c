#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "rdfa_utils.h"
#include "rdfa.h"

char* rdfa_replace_string(char* old_string, const char* new_string)
{
   size_t new_string_length = strlen(new_string);

   // free the memory associated with the old string if it exists.
   if(old_string != NULL)
   {
      free(old_string);
   }

   // copy the new string
   old_string = malloc(new_string_length);
   strcpy(old_string, new_string);

   return old_string;
}

char** rdfa_init_mapping(size_t elements)
{
   size_t mapping_size = sizeof(char*) * elements * 2;
   char** mapping = malloc(mapping_size);

   // only initialize the mapping if it is null.
   if(mapping != NULL)
   {
      memset(mapping, 0, mapping_size);
   }
   
   return mapping;
}

void rdfa_update_mapping(char** mapping, const char* key, const char* value)
{
   int found = 0;
   char** mptr = mapping;
   
   // search the current mapping to see if the namespace
   // prefix exists in the mapping
   while(*mptr != NULL)
   {
      if(strcmp(*mptr, key) == 0)
      {
         mptr++;
         *mptr = rdfa_replace_string(*mptr, value);
         found = 1;
      }
      else
      {
         mptr++;
      }
      mptr++;
   }

   // if we made it through the entire URI mapping and the key was not
   // found, create a new key-value pair.
   if(!found)
   {
      *mptr = rdfa_replace_string(*mptr, key);
      mptr++;
      *mptr = rdfa_replace_string(*mptr, value);
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

const char* rdfa_get_mapping(char** mapping, const char* key)
{
   return NULL;
}
