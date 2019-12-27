#include "Python.h"

#include <stdio.h>
#include <stdlib.h>
#include <iterator>
#include <dds/core/corefwd.hpp>
#include "PyOutput.hpp"

namespace rti { namespace routing { namespace py {
/*
 * --- PyOutput Python methods -------------------------------------------------
 */
PyObject* PyOutput::name(PyOutput *self, PyObject *Py_UNUSED(ignored))
{
    return PyUnicode_FromString(
            RTI_RoutingServiceRoute_get_output_name(self->native_route(), self->get()));
}

PyObject* PyOutput::write(PyOutput *self, PyObject *args)
{
    PyData *py_data = NULL;
    if (!PyArg_ParseTuple(
            args,
            "O",
            &py_data)) {
        return NULL;
    }

    const RTI_RoutingServiceSample out_samples[1] = {
        const_cast<void *> (reinterpret_cast<const void *> (py_data->get()))
    };
    const RTI_RoutingServiceSampleInfo out_infos[1] = {
        const_cast<void *> (reinterpret_cast<const void *> (NULL))
    };

    try {
        self->get()->write(
                self->get()->stream_writer_data,
                out_samples,
                NULL,
                1,
                self->native_env_);
        RTI_ROUTING_THROW_ON_ENV_ERROR(self->native_env_);
    } catch (const std::exception &ex) {
        PyErr_Format(PyExc_RuntimeError, "%s", ex.what());
    }

    Py_RETURN_NONE;
}


static PyMethodDef PyOutput_g_methods[] = {
    {
        "name",
        (PyCFunction) PyOutput::name,
        METH_NOARGS,
        "Return the name of this output"
    },
    {
        "write",
        (PyCFunction) PyOutput::write,
        METH_VARARGS,
        "write a sample by its data and info portion"
    },
    {NULL}  /* Sentinel */
};

static PyTypeObject PyOutput_g_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "pyproc.Output",
    .tp_doc = "Output object",
    .tp_basicsize = sizeof(PyOutput),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyNativeWrapper<PyOutputType>::new_object,
    .tp_dealloc = PyNativeWrapper<PyOutputType>::delete_object,
    .tp_methods = PyOutput_g_methods
};


PyOutput::PyOutput(
        RTI_RoutingServiceStreamWriterExt* native,
        RTI_RoutingServiceRoute *native_route,
        RTI_RoutingServiceEnvironment *environment)
        : PyNativeWrapper(native),
        native_route_(native_route),
        native_env_(environment)
{
}


RTI_RoutingServiceRoute* PyOutput::native_route()
{
    return native_route_;
}


PyTypeObject* PyOutputType::type()
{
    return &PyOutput_g_type;
}

const std::string& PyOutputType::name()
{
    static std::string __name("Output");

    return __name;
}



} } }

