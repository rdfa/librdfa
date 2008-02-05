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
void process_triple(rdftriple* triple)
{
   rdfa_print_triple(triple);
   rdfa_free_triple(triple);
}

size_t fill_buffer(char* buffer, size_t buffer_length)
{
   PyGILState_STATE state = PyGILState_Ensure();

   size_t rval = 0;
   PyObject* func;
   PyObject* dataFile = (PyObject*)gRdfaParser->mBufferFillerData;
   PyObject* arglist;
   PyObject* pyresult;

   // call the Python function to get the data for the buffer
   func = (PyObject*)gRdfaParser->mBufferFillerCallback;
   arglist = Py_BuildValue("(Oi)", dataFile, buffer_length);

   SWIG_PYTHON_THREAD_BEGIN_BLOCK;
   pyresult = PyEval_CallObject(func, arglist);
   SWIG_PYTHON_THREAD_END_BLOCK;
   Py_DECREF(arglist);

   // if we got a result, make sure that it is properly formatted and of the 
   // correct type.
   if(pyresult != NULL)
   {
      if(PyString_Check(pyresult))
      {
         // set the buffer if everything checks out
         char* sdata = PyString_AsString(pyresult);
         rval = PyString_Size(pyresult);
         strcpy(buffer, sdata);

         printf("BUFFER: %s\n", buffer);
      }
      else
      {
         PyErr_SetString(PyExc_TypeError, 
            "Buffer filler callback return type must be a string!");
      }
   }
   else
   {
      PyErr_SetString(PyExc_TypeError, 
            "Call to buffer filler failed from librdfa!");
   }
   Py_XDECREF(pyresult);

   PyGILState_Release(state);

   return rval;
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
   void setTripleHandler(PyObject *pyfunc) 
   {
      PyGILState_STATE state = PyGILState_Ensure();
      gRdfaParser = self;
      gRdfaParser->mTripleHandlerCallback = pyfunc;
      rdfa_set_triple_handler(gRdfaParser->mBaseContext, process_triple);
      Py_INCREF(pyfunc);
      PyGILState_Release(state);
   }

   /**
    * Sets the buffer filler callback for when more data is needed by
    * the XML parser when generating triples. Since librdfa is
    * stream-based, these reads usually happen in 4KB chunks.
    */
   void setBufferHandler(PyObject *pyfunc, PyObject* data) 
   {
      PyGILState_STATE state = PyGILState_Ensure();
      gRdfaParser = self;
      gRdfaParser->mBufferFillerCallback = pyfunc;
      gRdfaParser->mBufferFillerData = data;
      rdfa_set_buffer_filler(gRdfaParser->mBaseContext, &fill_buffer);
      Py_INCREF(pyfunc);
      PyGILState_Release(state);
   }
}
