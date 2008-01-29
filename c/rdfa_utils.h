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
 * A CURIE parse type lets the CURIE processor know what type of CURIE
 * is being parsed so that the proper namespace resolution may occur.
 */
typedef enum
{
   CURIE_PARSE_INSTANCEOF,
   CURIE_PARSE_RELREV,
   CURIE_PARSE_PROPERTY
} curieparse_t;

/**
 * The list member flag type is used to attach attribute information
 * to list member data.
 */
typedef enum
{
   RDFALIST_FLAG_NONE = 0,
   RDFALIST_FLAG_FORWARD = (1 << 1),
   RDFALIST_FLAG_REVERSE = (1 << 2),
   RDFALIST_FLAG_LAST = (1 << 3)
} liflag_t;

/**
 * Initializes a mapping given the number of elements the mapping is
 * expected to hold.
 *
 * @param elements the maximum number of elements the mapping is
 *                 supposed to hold.
 *
 * @return an initialized char**, with all of the elements set to NULL.
 */
char** rdfa_create_mapping(size_t elements);

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
 * Gets the value for a given mapping when presented with a key. If
 * the key doesn't exist in the mapping, NULL is returned.
 *
 * @param mapping the mapping to search.
 * @param key the key.
 *
 * @return value the value in the mapping for the given key.
 */
const char* rdfa_get_mapping(char** mapping, const char* key);

/**
 * Prints the mapping to the screen in a human-readable way.
 *
 * @param mapping the mapping to print to the screen.
 */
void rdfa_print_mapping(char** mapping);

/**
 * Frees all memory associated with a mapping.
 *
 * @param mapping the mapping to free.
 */
void rdfa_free_mapping(char** mapping);

/**
 * Creates a list and initializes it to the given size.
 */
rdfalist* rdfa_create_list(size_t size);

/**
 * Creates a list and initializes it to the given size.
 *
 * @param list the list to add the item to.
 * @param data the data to add to the list.
 * @param flags the flags to attach to the item.
 */
void rdfa_add_item(rdfalist* list, char* data, liflag_t flags);

/**
 * Prints the list to the screen in a human-readable way.
 *
 * @param list the list to print to the screen.
 */
void rdfa_print_list(rdfalist* list);

/**
 * Frees all memory associated with the given list.
 *
 * @param list the list to free.
 */
void rdfa_free_list(rdfalist* list);

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
 * Joins two strings together and returns a newly allocated string
 * with both strings joined.
 *
 * @param prefix the beginning part of the string.
 * @param suffix the ending part of the string.
 *
 * @return a pointer to the newly allocated string that has both
 *         prefix and suffix in it.
 */
char* rdfa_join_string(const char* prefix, const char* suffix);

/**
 * Creates a triple given the subject, predicate, object, datatype and
 * language for the triple.
 *
 * @param subject the subject for the triple.
 * @param predicate the predicate for the triple.
 * @param object the object for the triple.
 * @param datatype the datatype of the triple.
 * @param language the language for the triple.
 *
 * @return a newly allocated triple with all of the given
 *         information. This triple MUST be free()'d when you are done
 *         with it.
 */
rdftriple* rdfa_create_triple(const char* subject, const char* predicate,
   const char* object, const char* datatype, const char* language);

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
 * Resolves one or more CURIEs into fully qualified IRIs.
 *
 * @param rdfa_context the current processing context.
 * @param uris a list of URIs.
 * @param mode the CURIE parsing mode to use, one of
 *             CURIE_PARSE_INSTANCEOF, CURIE_PARSE_RELREV, or
 *             CURIE_PARSE_PROPERTY.
 *
 * @return an RDFa list if one or more IRIs were generated, NULL if not.
 */
rdfalist* rdfa_resolve_curie_list(
   rdfacontext* rdfa_context, const char* uris, curieparse_t mode);

#endif
