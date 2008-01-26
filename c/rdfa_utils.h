/**
 * This file contains functions used for common rdfa utility functions.
 */
#ifndef _RDFA_UTILS_H_
#define _RDFA_UTILS_H_
#include "rdfa.h"

/**
 * A CURIE type can be safe, unsafe, and Internationalized Resource
 * Identifier, reference-only or invalid.
 */
typedef enum
{
   CURIE_TYPE_SAFE,
   CURIE_TYPE_UNSAFE,
   CURIE_TYPE_IRI,
   CURIE_TYPE_REFERENCE,
   CURIE_TYPE_INVALID
}  curie_t;

/**
 * Initializes a mapping given the number of elements the mapping is
 * expected to hold.
 *
 * @param elements the maximum number of elements the mapping is
 *                 supposed to hold.
 *
 * @return an initialized char**, with all of the elements set to NULL.
 */
char** rdfa_init_mapping(size_t elements);

/**
 * Updates the given mapping when presented with a key and a value. If
 * the key doesn't exist in the mapping, it is created.
 *
 * @param mapping the mapping to update.
 * @param key the key.
 * @param value the value.
 */
void rdfa_update_mapping(char** mapping, const char* key, const char* value);

/**
 * Replaces an old string with a new string, freeing the old memory
 * and allocating new memory for the new string.
 *
 * @param old_string the old string to free and replace.
 * @param new_string the new string to copy to the old_string's
 *                   location.
 *
 * @return a pointer to the newly allocated string.
 */
char* rdfa_replace_string(char* old_string, const char* new_string);

/**
 * Resolves a given uri depending on whether or not it is a fully
 * qualified IRI, a CURIE, or a short-form XHTML reserved word.
 *
 * @param context the current processing context.
 * @param uri the URI part to process.
 *
 * @return the fully qualified IRI. The memory returned from this
 *         function MUST be freed.
 */
char* rdfa_resolve_curie(rdfacontext* context, const char* uri);

/**
 * Resolves a given uri depending on whether or not it is a fully
 * qualified IRI, a CURIE, or a short-form XHTML reserved word.
 *
 * @param context the current processing context.
 * @param uri the URI part to process.
 *
 * @return the fully qualified IRI, or NULL if the conversion failed
 *         due to the given URI not being a short-form XHTML reserved
 *         word. The memory returned from this function MUST be freed.
 */
char* rdfa_resolve_legacy_curie(rdfacontext* context, const char* uri);

#endif
