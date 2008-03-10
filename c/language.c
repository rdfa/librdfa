/**
 * Copyright 2008 Digital Bazaar, Inc.
 * This file is a part of librdfa and is licensed under the GNU LGPL v3.
 *
 * The language module is used to determine and set the current language.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rdfa_utils.h"
#include "rdfa.h"

/**
 * Updates the language given the value of the xml:lang attribute.
 *
 * @param lang the new value of the lang attribute.
 */
void rdfa_update_language(rdfacontext* context, const char* lang)
{
   // the [current element] is parsed for any language information,
   // and [language] is set in the [current evaluation context];

   if(lang != NULL)
   {
      context->language = rdfa_replace_string(context->language, lang);
   }
}
