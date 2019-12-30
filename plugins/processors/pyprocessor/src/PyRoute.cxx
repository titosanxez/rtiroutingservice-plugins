/* $Id$

 (c) Copyright, Real-Time Innovations, 2014-2016.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.
=========================================================================*/


#include "Python.h"

#include <stdio.h>
#include <stdlib.h>
#include <iterator>
#include <dds/core/corefwd.hpp>

#include "PyRoute.hpp"

namespace rti { namespace routing { namespace py {

/*
 * --- PyInputAccessor --------------------------------------------------------
 */
PyInputAccessor::PyInputAccessor(PyRoute* py_route)
        : py_route_(py_route),
        count_(RTI_RoutingServiceRoute_get_input_count(py_route->get())),
        iterator_(0)
{
}

Py_ssize_t PyInputAccessor::count(PyInputAccessor* self)
{
    return self->count_;
}

PyObject* PyInputAccessor::binary(
        PyInputAccessor* self,
        PyObject* key)
{
    PyObject *py_input;
    if (PyLong_Check(key)) {
        py_input = self->py_route_->input(PyLong_AsLong(key));
    } else if (PyUnicode_Check(key)) {
        py_input = self->py_route_->input(PyUnicode_AsUTF8(key));
    } else {
        PyErr_SetString(
                PyExc_KeyError,
                "PyInputAccessor::binary: key must be an string representing an input name "
                "or an integer representing an input index");
        return NULL;
    }

    if (py_input != Py_None) {
        Py_INCREF(py_input);
    }

    return py_input;
}


PyObject* PyInputAccessor::get_iterator(PyInputAccessor *self)
{
    return new PyInputAccessor(self->py_route_);
}

PyObject* PyInputAccessor::iterator_next(PyInputAccessor *self)
{
    if (self->iterator_ == self->count_) {
        return NULL;
    }
    PyObject *py_input = (PyObject *) self->py_route_->input(self->iterator_);
    Py_INCREF(py_input);
    ++self->iterator_;

    return py_input;
}

static PyMappingMethods PyInputAccessor_g_mapping = {
    .mp_length = (lenfunc) PyInputAccessor::count,
    .mp_subscript = (binaryfunc) PyInputAccessor::binary
};


static PyTypeObject PyInputAccessor_g_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "pyproc.InputAccessor",
    .tp_doc = "InputAccessor object",
    .tp_basicsize = sizeof(PyInputAccessor),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_dealloc = PyAllocatorGeneric<PyInputAccessorType, PyObject>::delete_object,
    .tp_as_mapping = &PyInputAccessor_g_mapping,
    .tp_iter = (getiterfunc)  PyInputAccessor::get_iterator,
    .tp_iternext = (iternextfunc) PyInputAccessor::iterator_next
};

PyTypeObject* PyInputAccessorType::type()
{
    return &PyInputAccessor_g_type;
}

const std::string& PyInputAccessorType::name()
{
    static std::string __name("InputAccessor");

    return __name;
}


/*
 * --- PyOutputAccessor --------------------------------------------------------
 */
PyOutputAccessor::PyOutputAccessor(PyRoute* py_route)
        : py_route_(py_route),
        count_(RTI_RoutingServiceRoute_get_output_count(py_route->get())),
        iterator_(0)
{
}

Py_ssize_t PyOutputAccessor::count(PyOutputAccessor* self)
{
    return self->count_;
}

PyObject* PyOutputAccessor::binary(
        PyOutputAccessor* self,
        PyObject* key)
{
    PyObject *py_output;
    if (PyLong_Check(key)) {
        py_output = self->py_route_->output(PyLong_AsLong(key));
    } else if (PyUnicode_Check(key)) {
        py_output = self->py_route_->output(PyUnicode_AsUTF8(key));
    } else {
        PyErr_SetString(
                PyExc_KeyError,
                "PyOutputAccessor::binary: key must be an string representing an output name "
                "or an integer representing an output index");
        return NULL;
    }

    if (py_output != Py_None) {
        Py_INCREF(py_output);
    }

    return py_output;
}


PyObject* PyOutputAccessor::get_iterator(PyOutputAccessor *self)
{
    return new PyOutputAccessor(self->py_route_);
}

PyObject* PyOutputAccessor::iterator_next(PyOutputAccessor *self)
{
    if (self->iterator_ == self->count_) {
        return NULL;
    }
    PyObject *py_output = (PyObject *) self->py_route_->output(self->iterator_);
    Py_INCREF(py_output);
    ++self->iterator_;

    return py_output;
}

static PyMappingMethods PyOutputAccessor_g_mapping = {
    .mp_length = (lenfunc) PyOutputAccessor::count,
    .mp_subscript = (binaryfunc) PyOutputAccessor::binary
};


static PyTypeObject PyOutputAccessor_g_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "pyproc.OutputAccessor",
    .tp_doc = "OutputAccessor object",
    .tp_basicsize = sizeof(PyOutputAccessor),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_dealloc = PyAllocatorGeneric<PyOutputAccessorType, PyObject>::delete_object,
    .tp_as_mapping = &PyOutputAccessor_g_mapping,
    .tp_iter = (getiterfunc)  PyOutputAccessor::get_iterator,
    .tp_iternext = (iternextfunc) PyOutputAccessor::iterator_next
};

PyTypeObject* PyOutputAccessorType::type()
{
    return &PyOutputAccessor_g_type;
}

const std::string& PyOutputAccessorType::name()
{
    static std::string __name("OutputAccessor");

    return __name;
}


/*
 * --- PyRoute Python methods -------------------------------------------------
 */

//PyObject* PyRoute::inputs(PyRoute *self, PyObject *args)
//{
//    PyObject *value = NULL;
//
//    if (!PyArg_ParseTuple(args, "O", &value)) {
//        return NULL;
//    }
//
//    PyInput *py_input = NULL;
//    try {
//        if (PyLong_Check(value)) {
//            py_input = self->input(PyLong_AsLong(value));
//        } else if (PyUnicode_Check(value)) {
//            py_input = self->input(PyUnicode_AsUTF8(value));
//        } else {
//            throw dds::core::InvalidArgumentError("inputs: invalid argument type");
//        }
//    } catch (const std::exception &ex) {
//        PyErr_Format(PyExc_IndexError, "%s", ex.what());
//        return NULL;
//    }
//
//     if (py_input != Py_None) {
//        Py_INCREF(py_input);
//    }
//    return py_input;
//}

Py_ssize_t PyRoute::port_count(PyRoute *self)
{
    return RTI_RoutingServiceRoute_get_input_count(self->get())
            +  RTI_RoutingServiceRoute_get_output_count(self->get());
}


PyObject* PyRoute::binary(PyRoute *self, PyObject *key)
{
    if (!PyUnicode_Check(key)) {
        PyErr_SetString(
                PyExc_KeyError,
                "PyRoute::binary: key must be an string represent an input or output name");
        return NULL;
    }

    /* try inputs first */
    PyObject *py_port = (PyObject *) self->input(PyUnicode_AsUTF8(key));
    if (py_port != Py_None) {
        Py_INCREF(py_port);
        return py_port;
    }

    /* try outputs */
    py_port = (PyObject *) self->output(PyUnicode_AsUTF8(key));
    if (py_port != Py_None) {
        Py_INCREF(py_port);
        return py_port;
    }

    return Py_None;
}

PyObject* PyRoute::in_accessor(PyRoute *self, void *)
{
    Py_INCREF(self->input_accessor_);
    return self->input_accessor_;
}

PyObject* PyRoute::out_accessor(PyRoute* self, void*)
{
    Py_INCREF(self->output_accessor_);
    return self->output_accessor_;
}


static PyGetSetDef PyRoute_getsetters[] = {
    {
        (char *) "inputs",
        (getter) PyRoute::in_accessor,
        (setter) NULL,
        (char *) "returns the input accessor",
        NULL
    },
    {
        (char *) "outputs",
        (getter) PyRoute::out_accessor,
        (setter) NULL,
        (char *) "returns the output accessor",
        NULL
    },
    {NULL}  /* Sentinel */
};

static PyMethodDef PyRoute_g_methods[] = {
//    {
//        "outputs",
//        (PyCFunction) PyRoute::outputs,
//        METH_VARARGS,
//        "returns the output at the specified index"
//    },
    {NULL}  /* Sentinel */
};

static PyMappingMethods PyRoute_g_mapping = {
    .mp_length = (lenfunc) PyRoute::port_count,
    .mp_subscript = (binaryfunc) PyRoute::binary
};



static PyTypeObject PyRoute_g_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "pyproc.Route",
    .tp_doc = "Route object",
    .tp_basicsize = sizeof(PyRoute),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_dealloc = PyNativeWrapper<PyRouteType>::delete_object,
    .tp_methods = PyRoute_g_methods,
    .tp_getset = PyRoute_getsetters,
    .tp_as_mapping = &PyRoute_g_mapping
};


/*
 * --- PyRoute class Implementation -------------------------------------------
 */


PyRoute::PyRoute(RTI_RoutingServiceRoute *native_route)
      : PyNativeWrapper(native_route),
        input_accessor_(new PyInputAccessor(this)),
        output_accessor_(new PyOutputAccessor(this))
{
}

PyInput* PyRoute::input(int32_t index)
{
    return input(RTI_RoutingServiceRoute_get_input_at(native_, index));
}

PyInput* PyRoute::input(const char *name)
{
    RTI_RoutingServiceStreamReaderExt *native_input =
            RTI_RoutingServiceRoute_lookup_input_by_name(native_, name);
    if (native_input != NULL) {
        return input(native_input);
    }

    return (PyInput *) Py_None;
}

PyInput* PyRoute::input(RTI_RoutingServiceStreamReaderExt *native_input)
{
    void *py_input = RTI_RoutingServiceRoute_get_stream_port_user_data(
            native_,
            native_input->stream_reader_data);
    assert(py_input != NULL);

    return static_cast<PyInput*>(py_input);
}


PyOutput* PyRoute::output(int32_t index)
{
     return output(RTI_RoutingServiceRoute_get_output_at(native_, index));
}


PyOutput* PyRoute::output(const char *name)
{
    RTI_RoutingServiceStreamWriterExt *native_output =
            RTI_RoutingServiceRoute_lookup_output_by_name(native_, name);
    if (native_output != NULL) {
        return output(native_output);
    }

    return (PyOutput*) Py_None;
}

PyOutput * PyRoute::output(RTI_RoutingServiceStreamWriterExt* native_output)
{
    void *py_output = RTI_RoutingServiceRoute_get_stream_port_user_data(
            native_,
            native_output->stream_writer_data);
    assert(py_output != NULL);

    return static_cast<PyOutput*>(py_output);
}


PyTypeObject* PyRouteType::type()
{
    return &PyRoute_g_type;
}

const std::string& PyRouteType::name()
{
    static std::string __name("Route");

    return __name;
}


} } }



