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

PyObject* PyRoute::input_at(PyRoute *self, PyObject *args)
{
    int32_t index = 0;
    if (!PyArg_ParseTuple(args, "i", &index)) {
        return NULL;
    }

    PyInput *py_input = NULL;
    try {
        py_input = self->input(index);
    } catch (const std::exception &ex) {
        PyErr_Format(PyExc_IndexError, "%s", ex.what());
        return NULL;
    }

    Py_INCREF(py_input);
    return py_input;
}

PyObject* PyRoute::output_at(PyRoute* self, PyObject* args)
{
    int32_t index = 0;
    if (!PyArg_ParseTuple(args, "i", &index)) {
        return NULL;
    }

    PyOutput *py_output = NULL;
    try {
        py_output = self->output(index);
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
        "input",
        (PyCFunction) PyRoute::input_at,
        METH_VARARGS,
        "returns the input at the specified index"
    },
    {
        "output",
        (PyCFunction) PyRoute::output_at,
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
    .tp_new = PyNativeWrapper<PyRouteType>::new_object,
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


} } }



