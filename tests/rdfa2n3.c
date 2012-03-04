/*
 * Copyright 2008 Digital Bazaar, Inc.
 *
 * This file is part of librdfa.
 *
 * librdfa is Free Software, and can be licensed under any of the
 * following three licenses:
 *
 *   1. GNU Lesser General Public License (LGPL) V2.1 or any
 *      newer version
 *   2. GNU General Public License (GPL) V2 or any newer version
 *   3. Apache License, V2.0 or any newer version
 *
 * You may not use this file except in compliance with at least one of
 * the above three licenses.
 *
 * See LICENSE-* at the top of this software distribution for more
 * information regarding the details of each license.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with librdfa. If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <string.h>
#include <rdfa.h>
#include <rdfa_utils.h>

#define BASE_URI \
   "http://rdfa.info/test-suite/tests-cases/"

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

         rdfa_set_default_graph_triple_handler(context, &process_triple);
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
