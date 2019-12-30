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
 * --- PyRoute Python methods -------------------------------------------------
 */
PyObject* PyRoute::name(PyRoute *self, PyObject *Py_UNUSED(ignored))
{
    return PyUnicode_FromFormat("%s", "I'm a Python Route!");
}

PyObject* PyRoute::inputs(PyRoute *self, PyObject *args)
{
    PyObject *value = NULL;

    if (!PyArg_ParseTuple(args, "O", &value)) {
        return NULL;
    }

    PyInput *py_input = NULL;
    try {
        if (PyLong_Check(value)) {
            py_input = self->input(PyLong_AsLong(value));
        } else if (PyUnicode_Check(value)) {
            py_input = self->input(PyUnicode_AsUTF8(value));
        } else {
            throw dds::core::InvalidArgumentError("inputs: invalid argument type");
        }
    } catch (const std::exception &ex) {
        PyErr_Format(PyExc_IndexError, "%s", ex.what());
        return NULL;
    }

    Py_INCREF(py_input);
    return py_input;
}

PyObject* PyRoute::outputs(PyRoute* self, PyObject* args)
{
    PyObject *value = NULL;

    if (!PyArg_ParseTuple(args, "O", &value)) {
        return NULL;
    }


    PyOutput *py_output = NULL;
    try {
        if (PyLong_Check(value)) {
             py_output = self->output(PyLong_AsLong(value));
        } else if (PyUnicode_Check(value)) {
            py_output = self->output(PyUnicode_AsUTF8(value));
        } else {
            throw dds::core::InvalidArgumentError("outputs: invalid argument type");
        }
    } catch (const std::exception &ex) {
        PyErr_Format(PyExc_IndexError, "%s", ex.what());
        return NULL;
    }

    Py_INCREF(py_output);
    return py_output;
}


static PyMethodDef PyRoute_g_methods[] = {
    {
        "name",
        (PyCFunction) PyRoute::name,
        METH_NOARGS,
        "Return the name, combining the first and last name"
    },
    {
        "inputs",
        (PyCFunction) PyRoute::inputs,
        METH_VARARGS,
        "returns the input at the specified index"
    },
    {
        "outputs",
        (PyCFunction) PyRoute::outputs,
        METH_VARARGS,
        "returns the output at the specified index"
    },
    {NULL}  /* Sentinel */
};

static PyTypeObject PyRoute_g_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "pyproc.Route",
    .tp_doc = "Route object",
    .tp_basicsize = sizeof(PyRoute),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_dealloc = PyNativeWrapper<PyRouteType>::delete_object,
    .tp_methods = PyRoute_g_methods
};


/*
 * --- PyRoute class Implementation -------------------------------------------
 */


PyRoute::PyRoute(RTI_RoutingServiceRoute *native_route)
    : PyNativeWrapper(native_route)
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

    throw dds::core::InvalidArgumentError(
            "inputs: input with name=" + std::string(name) + " not found");
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

    throw dds::core::InvalidArgumentError(
            "outputs: output with name=" + std::string(name) + " not found");
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



