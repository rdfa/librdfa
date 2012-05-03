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
 * This test checks to see how quickly we can process triples and is a
 * very basic performance test for the librdfa library.
 */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <rdfa.h>
#include <rdfa_utils.h>

#define MAX_ITERATIONS 20000
int g_iteration = 0;
rdfacontext* g_context = NULL;
size_t g_bytes_processed = 0;

static void process_triple(rdftriple* triple, void* callback_data)
{
   rdfa_free_triple(triple);
}

static size_t fill_buffer(char* buffer, size_t buffer_length, void* callback_data)
{
   char* data = NULL;
   size_t data_length = buffer_length;

   /* short-circuit last iteration */
   if(g_iteration == MAX_ITERATIONS + 1)
   {
      return 0;
   }

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
   clock_t stime;
   clock_t etime;
   rdfacontext* g_context;
   float delta;

   printf("Speed test...\n");

   stime = clock();

   g_context = rdfa_create_context("http://example.org/speed");
   rdfa_set_default_graph_triple_handler(g_context, &process_triple);
   rdfa_set_buffer_filler(g_context, &fill_buffer);
   rdfa_parse(g_context);
   rdfa_free_context(g_context);

   etime = clock();

   delta = etime - stime;
   printf("Processed %1.2f triples per second from %lu bytes of data.\n",
          (MAX_ITERATIONS / (delta / CLOCKS_PER_SEC)), (unsigned long)g_bytes_processed);

   return 0;
}
