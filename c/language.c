/**
 * The language module is used to determine and set the current language.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rdfa.h"

/**
 * Updates the language given the value of the xml:lang attribute.
 *
 * @param xml_lang the value of the xml_lang attribute.
 */
void rdfa_update_language(rdfacontext* context, const char* xml_lang)
{
   // the [current element] is parsed for any language information,
   // and [language] is set in the [current evaluation context];

   if(xml_lang != NULL)
   {
      // Language information can be provided using the general-purpose
      // XMLattribute @xml:lang.
      size_t xml_lang_length = strlen(xml_lang);
      if(context->language != NULL)
      {
         free(context->language);
      }
      context->language = malloc(xml_lang_length);
      strcpy(context->language, xml_lang);
   }
}
