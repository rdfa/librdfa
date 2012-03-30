/*
 * Copyright 2010 Digital Bazaar, Inc.
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
 * This test checks to see how quickly we can process triples and is a
 * very basic performance test for the librdfa library.
 *
 * Note: This speed test is different from the other speed test in that it uses
 * the alternate parsing API.
 */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <rdfa.h>
#include <rdfa_utils.h>

#define MAX_ITERATIONS 20000
int g_iteration = 0;
unsigned long long g_bytes_processed = 0;

static void process_triple(rdftriple* triple, void* callback_data)
{
   rdfa_free_triple(triple);
}

static size_t fill_buffer(rdfacontext* context)
{
   char* buffer = NULL;
   size_t buffer_length = 0;
   const char* data = NULL;
   size_t data_length = 0;

   /* get buffer to fill */
   buffer = rdfa_get_buffer(context, &buffer_length);
   data_length = buffer_length;
   memset(buffer, ' ', buffer_length);

   /* Note: code assumes data length < buffer length */
   if(g_iteration == 0)
   {
      data = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
      "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML+RDFa 1.0//EN\" "
      "\"http://www.w3.org/MarkUp/DTD/xhtml-rdfa-1.dtd\">\n"
      "<html xmlns=\"http://www.w3.org/1999/xhtml\"\n"
      "      xmlns:dc=\"http://purl.org/dc/elements/1.1/\">\n"
      "<head><title>Speed Test</title></head>\n"
      "<body><p>\n";
      memcpy(buffer, data, strlen(data));
   }
   else if(g_iteration < MAX_ITERATIONS)
   {
      data = "<span about=\"#foo\" rel=\"dc:title\" resource=\"#you\" />";
      memcpy(buffer, data, strlen(data));
   }
   else
   {
      data = "</p></body></html>";

      /* update data_length because this is the end of the stream, no
       * whitespace after it */
      data_length = strlen(data);
      memcpy(buffer, data, data_length);
   }

   g_iteration++;
   g_bytes_processed += data_length;
   /*buffer[buffer_length - 1] = 0;*/

   /*printf("%s", buffer);*/

   return data_length;
}

int main(int argc, char** argv)
{
   size_t bytes;
   clock_t stime;
   clock_t etime;
   rdfacontext* context;
   float delta;

   printf("Speed test...\n");

   stime = clock();

   context = rdfa_create_context("http://example.org/speed");
   rdfa_set_default_graph_triple_handler(context, &process_triple);
   rdfa_parse_start(context);
   while(g_iteration <= MAX_ITERATIONS)
   {
      bytes = fill_buffer(context);
      rdfa_parse_buffer(context, bytes);
   }
   rdfa_parse_buffer(context, 0);
   rdfa_parse_end(context);
   rdfa_free_context(context);

   etime = clock();

   delta = etime - stime;
   printf("Processed %1.2f triples per second from %lli bytes of data.\n",
          (MAX_ITERATIONS / (delta / CLOCKS_PER_SEC)), g_bytes_processed);

   return 0;
}
