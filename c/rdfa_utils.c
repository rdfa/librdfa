#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "rdfa_utils.h"
#include "rdfa.h"

char* rdfa_join_string(const char* prefix, const char* suffix)
{
   char* rval = NULL;
   size_t prefix_size = strlen(prefix);
   size_t suffix_size = strlen(suffix);
   rval = malloc(prefix_size + suffix_size + 1);
   
   memcpy(rval, prefix, prefix_size);
   memcpy(rval+prefix_size, suffix, suffix_size + 1);
   

   return rval;
}

char* rdfa_replace_string(char* old_string, const char* new_string)
{
   char* rval = NULL;
   
   if(new_string != NULL)
   {
      size_t new_string_length = strlen(new_string);

      // free the memory associated with the old string if it exists.
      if(old_string != NULL)
      {
         free(old_string);
      }

      // copy the new string
      rval = malloc(new_string_length + 1);
      strcpy(rval, new_string);
   }
   
   return rval;
}

rdfalist* rdfa_create_list(size_t size)
{
   rdfalist* rval = malloc(sizeof(rdfalist));

   rval->max_items = size;
   rval->num_items = 0;
   rval->items = NULL;
   rval->items = realloc(rval->items, sizeof(rdfalistitem) * rval->max_items);

   return rval;
}

void rdfa_print_list(rdfalist* list)
{
   printf("[ ");

   int i;
   for(i = 0; i < list->num_items; i++)
   {
      if(i != 0)
      {
         printf(", ");
      }
      
      printf(list->items[i]->data);
   }

   printf(" ]\n");
}

void rdfa_free_list(rdfalist* list)
{
   if(list != NULL)
   {
      int i;
      for(i = 0; i < list->num_items; i++)
      {
         free(list->items[i]->data);
         free(list->items[i]);
      }

      free(list->items);
      free(list);
   }
}

void rdfa_add_item(rdfalist* list, char* data, liflag_t flags)
{
   rdfalistitem* item = malloc(sizeof(rdfalistitem));

   item->data = NULL;
   item->data = rdfa_replace_string(item->data, data);
   item->flags = flags;

   if(list->num_items == list->max_items)
   {
      list->max_items = 1 + (list->max_items * 2);
      list->items =
         realloc(list->items, sizeof(rdfalistitem) * list->max_items);
   }

   list->items[list->num_items] = item;
   list->num_items++;
}

rdftriple* rdfa_create_triple(const char* subject, const char* predicate,
   const char* object, const char* datatype, const char* language)
{
   rdftriple* rval = malloc(sizeof(rdftriple));

   // clear the memory
   rval->subject = NULL;
   rval->predicate = NULL;
   rval->object = NULL;
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

char** rdfa_create_mapping(size_t elements)
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

   rdfa_print_mapping(mapping);
}

const char* rdfa_get_mapping(char** mapping, const char* key)
{
   const char* rval = NULL;
   char** mptr = mapping;
   
   // search the current mapping to see if the key exists in the mapping.
   while(*mptr != NULL)
   {
      if(strcmp(*mptr, key) == 0)
      {
         mptr++;
         rval = *mptr;
      }
      else
      {
         mptr++;
      }
      mptr++;
   }
   
   return rval;
}

void rdfa_print_mapping(char** mapping)
{
   char** mptr = mapping;
   printf("{\n");
   while(*mptr != NULL)
   {
      char* key = *mptr;
      mptr++;
      char* value = *mptr;
      mptr++;

      printf("   %s : %s", key, value);
      if(*mptr != NULL)
      {
         printf(",\n");
      }
      else
      {
         printf("\n");
      }
   }
   printf("}\n");
}

void rdfa_free_mapping(char** mapping)
{
   char** mptr = mapping;

   if(mapping != NULL)
   {
      // free all of the memory in the mapping
      while(*mptr != NULL)
      {
         free(*mptr);
         mptr++;
      }

      free(mapping);
   }
}
