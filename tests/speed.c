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
#include <time.h>
#include <rdfa.h>
#include <rdfa_utils.h>

#define MAX_ITERATIONS 1000
int g_iteration = 0;
rdfacontext* g_context = NULL;
unsigned long long g_bytes_processed = 0;

void process_triple(rdftriple* triple, void* callback_data)
{
   rdfa_free_triple(triple);
}

size_t fill_buffer(char* buffer, size_t buffer_length, void* callback_data)
{
   char* data = NULL;
   size_t data_length = buffer_length;

   if(g_iteration == 0)
   {
      data = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
      "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML+RDFa 1.0//EN\" "
      "\"http://www.w3.org/MarkUp/DTD/xhtml-rdfa-1.dtd\">\n"
      "<html xmlns=\"http://www.w3.org/1999/xhtml\"\n"
      "      xmlns:dc=\"http://purl.org/dc/elements/1.1/\">\n"
      "<head><title>Speed Test</title></head>\n"
      "<body><p>\n";
      memset(buffer, ' ', buffer_length);
      memcpy(buffer, data, strlen(data));
   }
   else if(g_iteration < MAX_ITERATIONS)
   {
      data = "<span about=\"#foo\" rel=\"dc:title\" resource=\"#you\" />";
      memset(buffer, ' ', buffer_length);
      memcpy(buffer, data, strlen(data));
   }
   else
   {
      data = "</p></body></html>";
      memset(buffer, ' ', buffer_length);
      memcpy(buffer, data, strlen(data));
      data_length = strlen(data);
   }
   
   g_iteration++;
   g_bytes_processed += data_length;
   //buffer[buffer_length - 1] = 0;

   //printf("%s", buffer);
   
   return data_length;
}

int main(int argc, char** argv)
{
   printf("Speed test...\n");

   clock_t stime = clock();
   
   rdfacontext* g_context = rdfa_create_context("http://example.org/speed");
   rdfa_set_triple_handler(g_context, &process_triple);
   rdfa_set_buffer_filler(g_context, &fill_buffer);
   rdfa_parse(g_context);
   rdfa_free_context(g_context);

   clock_t etime = clock();

   float delta = etime - stime;
   printf("Processed %1.2f triples per second from %lli bytes of data.\n",
          (MAX_ITERATIONS / (delta / CLOCKS_PER_SEC)), g_bytes_processed);
   
   return 0;
}
