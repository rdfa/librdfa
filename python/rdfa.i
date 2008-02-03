%module rdfa
%{
#include "stdlib.h"
#include "RdfaParser.h"
#include "rdfa_utils.h"
#include "rdfa.h"

RdfaParser* gRdfaParser = NULL;
void process_triple(rdftriple* triple);
size_t fill_buffer(char* buffer, size_t buffer_length);

%}

// Grab a Python function object as a Python object.
#ifdef SWIGPYTHON
%typemap(in) PyObject *pyfunc {
  if (!PyCallable_Check($input)) {
      PyErr_SetString(PyExc_TypeError, "Need a callable object!");
      return NULL;
  }
  $1 = $input;
}
#endif

%include RdfaParser.h

%{
/* This function matches the prototype of the normal C callback
   function for our widget. However, we use the clientdata pointer
   for holding a reference to a Python callable object. */

static double PythonCallBack(double a, void *clientdata)
{
   PyObject *func, *arglist;
   PyObject *result;
   double    dres = 0;
   
   func = (PyObject *) clientdata;               // Get Python function
   arglist = Py_BuildValue("(d)",a);             // Build argument list
   result = PyEval_CallObject(func,arglist);     // Call Python
   Py_DECREF(arglist);                           // Trash arglist
   if (result) {                                 // If no errors, return double
     dres = PyFloat_AsDouble(result);
   }
   Py_XDECREF(result);
   return dres;
}

   void process_triple(rdftriple* triple)
   {  
      printf("rdfa.i - process_triple...");

      rdfa_print_triple(triple);
      rdfa_free_triple(triple);
   }

   size_t fill_buffer(char* buffer, size_t buffer_length)
   {

      char* data = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
      "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML+RDFa 1.0//EN\" "
      "\"http://www.w3.org/MarkUp/DTD/xhtml-rdfa-1.dtd\">\n"
      "<html xmlns=\"http://www.w3.org/1999/xhtml\"\n"
      "      xmlns:dc=\"http://purl.org/dc/elements/1.1/\">\n"
      "<head><title>Speed Test</title></head>\n"
      "<body><p>\n"
      "<span about=\"#foo\" rel=\"dc:title\" resource=\"#you\" />\n"
      "</p></body></html>";

      size_t data_length = strlen(data);

      memset(buffer, 0, buffer_length);
      memcpy(buffer, data, strlen(data));

      return data_length;
   }
%}

// Attach a new method to our plot widget for adding Python functions
%extend RdfaParser {
   // Set a Python function object as a callback function
   // Note : PyObject *pyfunc is remapped with a typempap

   /**
    * Sets the triple handler callback for when triples are generated
    * by librdfa and need to be passed up to the application for
    * processing.
    */
   void setTripleHandler(PyObject *pyfunc) {
     //self->set_method(PythonCallBack, (void *)pyfunc);
     gRdfaParser = self;
     rdfa_set_triple_handler(gRdfaParser->mBaseContext, process_triple);
     Py_INCREF(pyfunc);
   }

   /**
    * Sets the buffer filler callback for when more data is needed by
    * the XML parser when generating triples. Since librdfa is
    * stream-based, these reads usually happen in 4KB chunks.
    */
   void setBufferHandler(PyObject *pyfunc) {
     //self->set_method(PythonCallBack, (void *)pyfunc);
     gRdfaParser = self;
     rdfa_set_buffer_filler(gRdfaParser->mBaseContext, &fill_buffer);
     Py_INCREF(pyfunc);
   }
}
