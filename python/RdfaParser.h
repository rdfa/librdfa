/*
 * Copyright (c) 2008 Digital Bazaar, Inc.
 *
 * This file is part of librdfa.
 *
 * librdfa is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * librdfa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with librdfa. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _RDFA_PARSER_H_
#define _RDFA_PARSER_H_

#include <string>

struct rdfacontext;

#define ACQUIRE_GIL PyGILState_STATE state; if(gRdfaParser->mUseGil) state = PyGILState_Ensure()
#define RELEASE_GIL if(gRdfaParser->mUseGil) PyGILState_Release(state)

/**
 * The RdfaParser class is a wrapper class for Python to provide a
 * simple API for using librdfa in Python.
 */
class RdfaParser
{
public:
   /**
    * The base URI that will be used when resolving relative pathnames
    * in the document.
    */
   std::string mBaseUri;

   /**
    * The base RDFa context to use when setting the triple handler callback,
    * buffer filler callback, and executing the parser call.
    */
   rdfacontext* mBaseContext;

   /**
    * The language-specific callback for filling the buffer.
    */
   void* mBufferFillerCallback;

   /**
    * The language-specific callback data needed to fill the buffer.
    */
   void* mBufferFillerData;

   /**
    * The language-specific callback for handling newly created triples
    * from the default graph.
    */
   void* mDefaultGraphTripleHandlerCallback;

   /**
    * The language-specific callback for handling processor graph created 
    * triples.
    */
   void* mProcessorGraphTripleHandlerCallback;

   /**
    * The language-specific callback data needed to handle the triple.
    */
   void* mTripleHandlerData;
   
   /**
    * Flag to specify whether or not to use the Global Interpreter Lock.
    */
   bool mUseGil;

   /**
    * Standard constructor for the RDFa parser.
    *
    * @param baseUri the base URI to use when resolving relative
    *                pathnames in the document. This value should be
    *                the fully qualified URI to the document.
    * @param useGil uses the Python Global Interpreter Lock. Turned on by
    *               default, but there may be some places where you don't want
    *               to use this, like from within Apache.
    */
   RdfaParser(const char* baseUri, bool useGil = 1);

   /**
    * Standard destructor.
    */
   ~RdfaParser();

   /**
    * Starts the parsing process for librdfa. When more data is
    * required by the XML parser, the buffer filler callback is
    * called. If triples are found, then the triple handler callback
    * is called.
    */
   int parse();
};

#endif /* _RDFA_PARSER_H_ */

