/*
 * Copyright 2008 Digital Bazaar, Inc.
 * This file is a part of librdfa and is licensed under the GNU LGPL v3.
 */
#include <stdio.h>
#include <string.h>
#include <rdfa.h>
#include <rdfa_utils.h>

#define BASE_URI \
   "http://www.w3.org/2006/07/SWD/RDFa/testsuite/xhtml1-testcases/"

FILE* g_xhtml_file = NULL;

void process_triple(rdftriple* triple)
{
   rdfa_print_triple(triple);
   rdfa_free_triple(triple);
}

size_t fill_buffer(char* buffer, size_t buffer_length)
{
   return fread(buffer, sizeof(char), buffer_length, g_xhtml_file);
}

int main(int argc, char** argv)
{
   if(argc < 2)
   {
      printf("%s usage:\n\n"
             "%s <input.xhtml>\n", argv[0], argv[0]);
   }
   else
   {
      g_xhtml_file = fopen(argv[1], "r");
      char* filename = rindex(argv[1], '/');
      filename++;
      
      if(g_xhtml_file != NULL)
      {
         char* base_uri = rdfa_join_string(BASE_URI, filename);
         rdfacontext* context = rdfa_create_context(base_uri);
         
         rdfa_set_triple_handler(context, &process_triple);
         rdfa_set_buffer_filler(context, &fill_buffer);
         rdfa_parse(context);
         rdfa_free_context(context);

         fclose(g_xhtml_file);
         free(base_uri);
      }
      else
      {
         perror("failed to open file:");
      }
   }
   
   return 0;
}
