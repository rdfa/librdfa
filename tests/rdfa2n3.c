/*
 * Copyright 2008 Digital Bazaar, Inc.
 *
 * This file is part of librdfa.
 *
 * librdfa is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * librdfa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with librdfa. If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <string.h>
#include <rdfa.h>
#include <rdfa_utils.h>

#define BASE_URI \
   "http://www.w3.org/2006/07/SWD/RDFa/testsuite/xhtml1-testcases/"

void process_triple(rdftriple* triple, void* callback_data)
{
   rdfa_print_triple(triple);
   rdfa_free_triple(triple);
}

size_t fill_buffer(char* buffer, size_t buffer_length, void* callback_data)
{
   FILE* xhtml_file = (FILE*)callback_data;
   size_t rval = fread(buffer, sizeof(char), buffer_length, xhtml_file);
   
   return rval;
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
      FILE* xhtml_file = fopen(argv[1], "r");
      char* filename = rindex(argv[1], '/');
      filename++;
      
      if(xhtml_file != NULL)
      {
         char* base_uri = rdfa_join_string(BASE_URI, filename);
         rdfacontext* context = rdfa_create_context(base_uri);
         context->callback_data = xhtml_file;
         
         rdfa_set_triple_handler(context, &process_triple);
         rdfa_set_buffer_filler(context, &fill_buffer);
         rdfa_parse(context);
         rdfa_free_context(context);
         
         fclose(xhtml_file);
         free(base_uri);
      }
      else
      {
         perror("failed to open file:");
      }
   }
   
   return 0;
}
