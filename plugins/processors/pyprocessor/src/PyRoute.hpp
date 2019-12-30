

#ifndef PYROUTE_HPP
#define PYROUTE_HPP

#include "Python.h"

#include <rti/routing/processor/Processor.hpp>
#include "NativeUtils.hpp"
#include "PyInput.hpp"
#include "PyOutput.hpp"


namespace rti { namespace routing { namespace py {

class PyRouteType
{
public:
    typedef RTI_RoutingServiceRoute native_type;
    static PyTypeObject* type();
    static const std::string& name();
};

class PyRoute : public PyNativeWrapper<PyRouteType>
{
public:
    PyRoute(RTI_RoutingServiceRoute *native);
    PyInput* input(RTI_RoutingServiceStreamReaderExt *native_input);
    PyInput* input(int32_t index);
    PyInput* input(const char *name);
    PyOutput* output(RTI_RoutingServiceStreamWriterExt *native_input);
    PyOutput* output(int32_t index);
    PyOutput* output(const char *name);
    static PyTypeObject* type();

    /* python methods */
    static
    PyObject* name(PyRoute *self, PyObject *Py_UNUSED(ignored));

    static
    PyObject* inputs(PyRoute *self, PyObject *args);

    static
    PyObject* outputs(PyRoute *self, PyObject *args);
};

}}}

#endif /* PYROUTE_HPP */

