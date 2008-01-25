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
 * rdfa_set_triple_handler(context, triple_function);
 * rdfa_set_buffer_filler(context, buffer_filler_function);
 * rdfa_parse(context);
 * rdfa_destroy_context(context);
 */
#ifndef _LIBRDFA_RDFA_H
#define _LIBRDFA_RDFA_H

#define RDFA_PARSE_FAILED -1
#define RDFA_PARSE_WARNING -2
#define RDFA_PARSE_SUCCESS 1

#define MAX_URI_MAPPINGS 512
#define MAX_INCOMPLETE_TRIPLES 1024

/**
 * An RDF triple is the result of an RDFa statement that contains, at
 * the very least, a subject, a predicate and an object. It is the
 * smallest, complete statement one can make in RDF.
 */
typedef struct rdftriple
{
   char* subject;
   char* predicate;
   char* object;
   char* datatype;
   char* language;
} rdftriple;

/**
 * The specification for a callback that is capable of handling
 * triples. Produces a triple that must be freed once the application
 * is done with the object.
 */
typedef void (*triple_handler_fp)(rdftriple*);

/**
 * The specification for a callback that is capable of handling
 * triples.
 */
typedef size_t (*buffer_filler_fp)(char*, size_t);

/**
 * The RDFa Parser structure is responsible for keeping track of the state of
 * the current RDFa parser. Things such as the default namespace, 
 * CURIE mappings, and other context-specific 
 */
typedef struct rdfacontext
{
   char* base;
   char* current_subject;
   char* parent_object;
   char* parent_bnode;
   char** uri_mappings;
   rdftriple** incomplete_triples;
   char* language;

   triple_handler_fp triple_callback;
   buffer_filler_fp buffer_filler_callback;
} rdfacontext;

/**
 * Creates an initial context for RDFa.
 *
 * @param base The base URI that should be used for the parser.
 *
 * @return a pointer to the base RDFa context, or NULL if memory
 *         allocation failed.
 */
rdfacontext* rdfa_create_context(const char* base);

/**
 * Sets the triple handler for the application.
 *
 * @param context the base rdfa context for the application.
 * @param th the triple handler function.
 */
void rdfa_set_triple_handler(rdfacontext* context, triple_handler_fp th);

/**
 * Sets the buffer filler for the application.
 *
 * @param context the base rdfa context for the application.
 * @param bf the buffer filler function.
 */
void rdfa_set_buffer_filler(rdfacontext* context, buffer_filler_fp bf);

/**
 * Starts processing given the base rdfa context.
 *
 * @param context the base rdfa context.
 *
 * @return RDFA_PARSE_SUCCESS if everything went well. RDFA_PARSE_FAILED
 *         if there was a fatal error and RDFA_PARSE_WARNING if there
 *         was a non-fatal error.
 */
int rdfa_parse(rdfacontext* context);

/**
 * Destroys the given rdfa context by freeing all memory associated
 * with the context.
 *
 * @param context the rdfa context.
 */
void rdfa_destroy_context(rdfacontext* context);

#endif
