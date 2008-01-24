/**
 * The librdfa library is the Fastest RDFa Parser in the Universe. It is
 * a stream parser, meaning that it takes an XML data as input and spits
 * out RDF triples as it comes across them in the stream. Due to this
 * processing approach, librdfa has a very, very small memory footprint.
 * It is also very fast and can operate on hundreds of gigabytes of XML
 * data without breaking a sweat.
 *
 * Usage:
 *
 * rdfacontext* context = rdfa_create_context(base_uri);
 * rdfa_set_triple_handler(context, func);
 * rdfa_set_buffer_filler(context, func);
 * rdfa_parse(context);
 * rdfa_destroy_parser(context);
 *
 * @author Manu Sporny
 */
#include <rdfa.h>
#include <stdlib.h>

rdfacontext* rdfa_create_context(const char* base_uri)
{
   return NULL;
}

void rdfa_set_triple_handler(rdfacontext* context, void* th)
{
}

void rdfa_set_buffer_filler(rdfacontext* context, void* bf)
{
}

int rdfa_parse(rdfacontext* context)
{
   return RDFA_PARSE_FAILED;
}

void rdfa_destroy_context(rdfacontext* context)
{
}
