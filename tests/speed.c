/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <rdfa.h>
#include <rdfa_utils.h>

#define MAX_ITERATIONS 1000000
int g_iteration = 0;
rdfacontext* g_context = NULL;

void process_triple(rdftriple* triple)
{
   rdfa_free_triple(triple);
}

size_t fill_buffer(char* buffer, size_t buffer_length)
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
      memcpy(buffer, data, 52);
   }
   else
   {
      data = "</p></body></html>";
      memset(buffer, ' ', buffer_length);
      memcpy(buffer, data, strlen(data));
      data_length = strlen(data);
   }
   
   g_iteration++;
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
   printf("Processed %1.2f triples per second.\n",
      MAX_ITERATIONS / (delta / CLOCKS_PER_SEC));
   
   return 0;
}
