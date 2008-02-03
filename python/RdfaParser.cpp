#include "rdfa.h"
#include "RdfaParser.h"

RdfaParser::RdfaParser(const char* baseUri)
{
   mBaseUri = baseUri;
   mBaseContext = rdfa_create_context(baseUri);
}

RdfaParser::~RdfaParser()
{
   rdfa_free_context(mBaseContext);
}

int RdfaParser::parse()
{
   return rdfa_parse(mBaseContext);
}
