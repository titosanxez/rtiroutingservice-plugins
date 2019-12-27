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
static
PyObject* PyRoute_name(PyRoute *self, PyObject *Py_UNUSED(ignored))
{
    return PyUnicode_FromFormat("%s", "I'm a Python Route!");
}

static
PyObject* PyRoute_input(PyRoute *self, PyObject *args)
{
    using dds::core::xtypes::DynamicData;

    int32_t index = 0;
    if (!PyArg_ParseTuple(args, "i", &index)) {
        return NULL;
    }

    PyInput *py_input = NULL;
    try {
        py_input = self->input(index);
    } catch (const std::exception &ex) {
        PyErr_Format(PyExc_RuntimeError, "%s", ex.what());
        return NULL;
    }

    Py_INCREF(py_input);
    return py_input;
}

static PyMethodDef PyRoute_g_methods[] = {
    {
        "name",
        (PyCFunction) PyRoute_name,
        METH_NOARGS,
        "Return the name, combining the first and last name"
    },
    {
        "input",
        (PyCFunction) PyRoute_input,
        METH_VARARGS,
        "returns the input at the specified index"
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

PyTypeObject* PyRouteType::type()
{
    return &PyRoute_g_type;
}


} } }



