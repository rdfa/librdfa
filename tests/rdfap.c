/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include <stdio.h>
#include <rdfa.h>

#define BASE_URI \
   "http://www.w3.org/2006/07/SWD/RDFa/testsuite/xhtml1-testcases/"

FILE* g_xhtml_file = NULL;

void process_triple(rdftriple* triple)
{
   printf("triple_handler_func\n");
}

size_t fill_buffer(char* buffer, size_t buffer_length)
{
   printf("fill_buffer\n");

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
      printf("Processing %s...\n", argv[1]);

      g_xhtml_file = fopen(argv[1], "r");

      if(g_xhtml_file != NULL)
      {
         rdfacontext* context = rdfa_create_context(BASE_URI);
         rdfa_set_triple_handler(context, &process_triple);
         rdfa_set_buffer_filler(context, &fill_buffer);
         rdfa_parse(context);
         rdfa_destroy_context(context);

         fclose(g_xhtml_file);
      }
      else
      {
         perror("failed to open file:");
      }
   }
   
   return 0;
}
