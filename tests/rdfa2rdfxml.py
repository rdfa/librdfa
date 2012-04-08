#!/usr/bin/env python
# 
# This file is part of librdfa.
# 
# librdfa is Free Software, and can be licensed under any of the
# following three licenses:
# 
#   1. GNU Lesser General Public License (LGPL) V2.1 or any 
#      newer version
#   2. GNU General Public License (GPL) V2 or any newer version
#   3. Apache License, V2.0 or any newer version
# 
# You may not use this file except in compliance with at least one of
# the above three licenses.
# 
# See LICENSE-* at the top of this software distribution for more
# information regarding the details of each license.
# 
# Reads in an XHTML+RDFa file and outputs the triples generated by the file
# in N3 format.
import sys, os, urllib2
sys.path += ("../python/dist",)
import rdfa
from StringIO import StringIO
from rdflib import Namespace, BNode, Literal, URIRef, Graph, ConjunctiveGraph

URL_TYPE_HTTP = 1
URL_TYPE_FILE = 2

##
# Called whenever a triple is generated by the underlying implementation for the
# default graph.
#
# @param rdf the rdf object to use when storing data.
# @param subject the subject of the triple.
# @param predicate the predicate for the triple.
# @param obj the object of the triple.
# @param objectType the type for the object in the triple.
# @param dataType the dataType for the object in the triple.
# @param language the language for the object in the triple.
def defaultTriple(rdf, subject, predicate, obj, objectType, dataType,
                  language):
    rdf['triples'].append(
        (subject, predicate, obj, objectType, dataType, language))
        
##
# Called whenever a triple is generated by the underlying implementation for the
# processor graph.
#
# @param rdf the rdf object to use when storing data.
# @param subject the subject of the triple.
# @param predicate the predicate for the triple.
# @param obj the object of the triple.
# @param objectType the type for the object in the triple.
# @param dataType the dataType for the object in the triple.
# @param language the language for the object in the triple.
def processorTriple(rdf, subject, predicate, obj, objectType, dataType,
                  language):
    rdf['processor_triples'].append(
        (subject, predicate, obj, objectType, dataType, language))

##
# Called whenever the processing buffer for the C-side needs to be re-filled.
#
# @param dataFile the file-like object to use when reading in the data stream.
# @param bufferSize the size of the buffer to return. Returning anything less
#                   than bufferSize will halt execution after the returned
#                   buffer has been processed.
def fillBuffer(dataFile, bufferSize):
    return dataFile.read()

##
# Gets RDF/XML given an object with pre-defined namespaces and triples.
#
# @param rdf the rdf dictionary object that contains namespaces and triples.
#
# @return the RDF/XML text.
def getRdfXml(rdf, graph):
    g = ConjunctiveGraph()
    n3 = ""
    
    # Append the RDF namespace and print the prefix namespace mappings
    for prefix, uri in rdf['namespaces'].items():
        g.bind(prefix, Namespace(uri))
    
    # BNode map
    bnodes = {}
    
    # Print each subject-based triple to the screen
    triples = rdf['triples']
    if(graph == "processor"):
        triples = rdf['processor_triples']
       
    for triple in triples:
        subject = triple[0]
        predicate = triple[1]
        obj = triple[2]
        objectType = triple[3]
        dataType = triple[4]
        language = triple[5]
        s = None
        p = None
        o = None
        
        # Create the subject
        if(subject.startswith("_:")):
            if(subject in bnodes):
                s = bnodes[subject]
            else:
                s = BNode()
                bnodes[subject] = s
        else:
            s = URIRef(subject)
        
        # Create the predicate
        p = URIRef(predicate)

        # Create the object
        if(objectType == rdfa.RDF_TYPE_IRI):
            if(obj.startswith("_:")):
                if(obj in bnodes):
                    o = bnodes[obj]
                else:
                    o = BNode()
                    bnodes[obj] = o
            else:
                o = URIRef(obj)
        elif(objectType == rdfa.RDF_TYPE_TYPED_LITERAL):
            o = Literal(obj, datatype = dataType)
        elif(objectType == rdfa.RDF_TYPE_PLAIN_LITERAL):
            if(language == None):
                o = Literal(obj)
            else:
                o = Literal(obj, lang = language)
        elif(objectType == rdfa.RDF_TYPE_XML_LITERAL):
            o = Literal(obj, datatype = 
                "http://www.w3.org/1999/02/22-rdf-syntax-ns#XMLLiteral")

        # Add the triple to the graph
        g.add((s, p, o))
    
    rdfxml = g.serialize(format="nt")

    return rdfxml

##
# The main entry point for the script.
#
# @param argv the argument list passed to the program.
# @param stdout the standard output stream assigned to the program.
# @param environ the execution environment for the program.
def main(argv, stdout, environ):
    urlType = URL_TYPE_FILE

    # Determine if the processor graph or the processor is being requested
    graph = "default"
    for arg in argv:
        if(arg == "-p" or arg == "--processor-graph"):
            graph = "processor"

    if((len(argv) > 1) and (len(argv[-1]) > 4)):
        if(argv[-1][:5] == "http:"):
            urlType = URL_TYPE_HTTP
    else:
        print "usage:", argv[0], "[-p | --processor-graph] <file>"
        print "or"
        print "      ", argv[0], "[-p | --processor-graph] <URL>"
        sys.exit(1)
    
    if((urlType == URL_TYPE_FILE) and (not os.path.exists(argv[-1]))):
        print "File %s, does not exist" % (argv[-1])
        sys.exit(1)
    if((urlType == URL_TYPE_FILE) and (not os.access(argv[-1], os.R_OK))):
        print "Cannot read file named %s" % (argv[-1])
        sys.exit(1)

    # Open the data file and setup the parser
    dataFile = None
    parser = None

    # Open the proper file stream and initialize the parser using the URL
    if(urlType == URL_TYPE_HTTP):
        dataFile = urllib2.urlopen(argv[-1])
        parser = rdfa.RdfaParser(argv[-1])
    else:
        dataFile = open(argv[-1], "r")
        parser = rdfa.RdfaParser("file://" + os.path.abspath(argv[-1]))

    # Create the RDF dictionary that will be used by the triple handler
    # callback
    rdf = {}
    rdf['namespaces'] = {}
    rdf['triples'] = []
    rdf['processor_triples'] = []

    # Setup the parser
    parser.setDefaultGraphTripleHandler(defaultTriple, rdf)
    parser.setProcessorGraphTripleHandler(processorTriple, rdf)
    parser.setBufferHandler(fillBuffer, dataFile)

    # Parse the document
    parser.parse()

    # Close the datafile
    dataFile.close()

    # Print the RDF/XML to stdout
    print getRdfXml(rdf, graph)

##
# Run the rdfa2n3 python application.
if __name__ == "__main__":
    main(sys.argv, sys.stdout, os.environ)
