/**
 * Copyright 2008 Digital Bazaar, Inc.
 *
 * This file is part of librdfa.
 *
 * librdfa is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 2 of the
 * License, or (at your option) any later version.
 *
 * librdfa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with librdfa. If not, see <http://www.gnu.org/licenses/>.
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
