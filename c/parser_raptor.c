/**
 * Copyright 2008-2011 Digital Bazaar, Inc.
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
 * The librdfa library is the Fastest RDFa Parser in the Universe. It is
 * a stream parser, meaning that it takes an XML data as input and spits
 * out RDF triples as it comes across them in the stream. Due to this
 * processing approach, librdfa has a very, very small memory footprint.
 * It is also very fast and can operate on hundreds of gigabytes of XML
 * data without breaking a sweat.
 */
#ifdef LIBRDFA_IN_RAPTOR

static int raptor_nspace_compare(const void *a, const void *b)
{
  raptor_namespace* ns_a=*(raptor_namespace**)a;
  raptor_namespace* ns_b=*(raptor_namespace**)b;
  if(!ns_a->prefix)
    return 1;
  else if(!ns_b->prefix)
    return -1;
  else
    return strcmp((const char*)ns_b->prefix, (const char*)ns_a->prefix);
}

static void raptor_rdfa_start_element(void *user_data,
                                      raptor_xml_element *xml_element)
{
  raptor_qname* qname=raptor_xml_element_get_name(xml_element);
  int attr_count=raptor_xml_element_get_attributes_count(xml_element);
  raptor_qname** attrs=raptor_xml_element_get_attributes(xml_element);
  unsigned char* qname_string=raptor_qname_to_counted_name(qname, NULL);
  char** attr=NULL;
  int i;

  if(attr_count > 0) {
    attr=(char**)malloc(sizeof(char*) * (1+(attr_count*2)));
    for(i=0; i<attr_count; i++) {
      attr[2*i]=(char*)raptor_qname_to_counted_name(attrs[i], NULL);
      attr[1+(2*i)]=(char*)raptor_qname_get_value(attrs[i]);
    }
    attr[2*i]=NULL;
  }
  start_element(user_data, (char*)qname_string, (const char**)attr);
  raptor_free_memory(qname_string);
  if(attr) {
    for(i=0; i<attr_count; i++)
      raptor_free_memory(attr[2*i]);
    free(attr);
  }
}

static void raptor_rdfa_end_element(void *user_data,
                                    raptor_xml_element* xml_element)
{
  raptor_qname* qname=raptor_xml_element_get_name(xml_element);
  unsigned char* qname_string=raptor_qname_to_counted_name(qname, NULL);

  end_element(user_data, (const char*)qname_string);
  raptor_free_memory(qname_string);
}

static void raptor_rdfa_character_data(void *user_data,
                                       raptor_xml_element* xml_element,
                                       const unsigned char *s, int len)
{
  character_data(user_data, (const char *)s, len);
}

static void raptor_rdfa_namespace_handler(void *user_data,
                                          raptor_namespace* nspace)
{
  rdfalist* context_stack = (rdfalist*)user_data;
  rdfacontext* context = (rdfacontext*)
    context_stack->items[context_stack->num_items - 1]->data;

  if(context->namespace_handler)
    (*context->namespace_handler)(context->namespace_handler_user_data,
                                  nspace);
}

#endif
