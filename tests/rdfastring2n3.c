/*
 * Copyright 2008 Digital Bazaar, Inc.
 *
 * This file is part of librdfa.
 *
 * librdfa is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 2 of the
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

FILE* g_xhtml_file = NULL;

/**
 * The buffer status struct is used to keep track of where we are in
 * the current buffer.
 */
typedef struct buffer_status
{
   char* buffer;
   unsigned int current_offset;
   unsigned int total_length;
} buffer_status;

void process_triple(rdftriple* triple, void* callback_data)
{
   rdfa_print_triple(triple);
   rdfa_free_triple(triple);
}

size_t fill_buffer(char* buffer, size_t buffer_length, void* callback_data)
{
   size_t rval = 0;
   buffer_status* bstatus = (buffer_status*)callback_data;

   if((bstatus->current_offset + buffer_length) < bstatus->total_length)
   {
      rval = buffer_length;
      memcpy(buffer, &bstatus->buffer[bstatus->current_offset], buffer_length);
      bstatus->current_offset += buffer_length;
   }
   else
   {
      rval = bstatus->total_length - bstatus->current_offset;
      memcpy(buffer, &bstatus->buffer[bstatus->current_offset], rval);
   }
   
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
      g_xhtml_file = fopen(argv[1], "r");
      char* filename = rindex(argv[1], '/');
      if(filename == NULL)
      {
         filename = argv[1];
      }
      else
      {
         filename++;
      }
      
      if(g_xhtml_file != NULL)
      {
         unsigned int buffer_length = 65535;
         char* buffer = malloc(buffer_length);
         char* base_uri = rdfa_join_string(BASE_URI, filename);
         rdfacontext* context = rdfa_create_context(base_uri);         
         buffer_status* status = malloc(sizeof(buffer_status));

         // get all of the buffer text
         fread(buffer, sizeof(char), buffer_length, g_xhtml_file);
         fclose(g_xhtml_file);

         // initialize the callback data
         status->buffer = buffer;
         status->current_offset = 0;
         status->total_length = strlen(buffer);
         context->callback_data = status;

         // setup the parser
         rdfa_set_triple_handler(context, &process_triple);
         rdfa_set_buffer_filler(context, &fill_buffer);
         rdfa_parse(context);
         rdfa_free_context(context);

         free(base_uri);
      }
      else
      {
         perror("failed to open file:");
      }
   }
   
   return 0;
}
