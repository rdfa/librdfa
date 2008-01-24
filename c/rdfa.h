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
 */

#define RDFA_PARSE_FAILED -1
#define RDFA_PARSE_WARNING -2
#define RDFA_PARSE_SUCCESS 1

/**
 * The RDFa Parser structure is responsible for keeping track of the state of
 * the current RDFa parser. Things such as the default namespace, 
 * CURIE mappings, and other context-specific 
 */
typedef struct rdfacontext
{
   char* base_uri; 
} rdfacontext;

rdfacontext* rdfa_create_context(const char* base_uri);
void rdfa_set_triple_handler(rdfacontext* context, void* th);
void rdfa_set_buffer_filler(rdfacontext* context, void* bf);
int rdfa_parse(rdfacontext* context);
void rdfa_destroy_context(rdfacontext* context);

