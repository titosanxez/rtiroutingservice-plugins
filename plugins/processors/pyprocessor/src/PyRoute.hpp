

#ifndef PYROUTE_HPP
#define PYROUTE_HPP

#include "Python.h"

#include <rti/routing/processor/Processor.hpp>
#include "NativeUtils.hpp"
#include "PyInput.hpp"
#include "PyOutput.hpp"


namespace rti { namespace routing { namespace py {


class PyRoute;

class PyInputAccessorType {

    public:
    static PyTypeObject* type();
    static const std::string& name();
};

class PyInputAccessor : public PyAllocatorGeneric<PyInputAccessorType, PyObject> {
public:
    PyInputAccessor(PyRoute *py_route);

    static
    Py_ssize_t count(PyInputAccessor *self);

    static
    PyObject* binary(PyInputAccessor *self, PyObject *key);

    static
    PyObject* get_iterator(PyInputAccessor *self);

    static
    PyObject* iterator_next(PyInputAccessor *self);

private:
    PyRoute *py_route_;
    int32_t count_;
    int32_t iterator_;
};


class PyOutputAccessorType {

    public:
    static PyTypeObject* type();
    static const std::string& name();
};

class PyOutputAccessor : public PyAllocatorGeneric<PyOutputAccessorType, PyObject> {
public:
    PyOutputAccessor(PyRoute *py_route);

    static
    Py_ssize_t count(PyOutputAccessor *self);

    static
    PyObject* binary(PyOutputAccessor *self, PyObject *key);

    static
    PyObject* get_iterator(PyOutputAccessor *self);

    static
    PyObject* iterator_next(PyOutputAccessor *self);

private:
    PyRoute *py_route_;
    int32_t count_;
    int32_t iterator_;
};

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
    PyObject* outputs(PyRoute *self, PyObject *args);

    static
    Py_ssize_t port_count(PyRoute *self);

    static
    PyObject* binary(PyRoute *self, PyObject *key);

    static
    PyObject* in_accessor(PyRoute *self, void *closure);

    static
    PyObject* out_accessor(PyRoute *self, void *closure);

private:
    PyInputAccessor *input_accessor_;
    PyOutputAccessor *output_accessor_;
};

}}}

#endif /* PYROUTE_HPP */

