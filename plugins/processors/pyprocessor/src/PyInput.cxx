#include "Python.h"

#include <stdio.h>
#include <stdlib.h>
#include <iterator>
#include <dds/core/corefwd.hpp>
#include "PyInput.hpp"

namespace rti { namespace routing { namespace py {
/*
 * --- PyInput Python methods -------------------------------------------------
 */
PyObject* PyInput::name(PyInput *self, PyObject *Py_UNUSED(ignored))
{
    return PyUnicode_FromString(
            RTI_RoutingServiceRoute_get_input_name(self->native_route(), self->get()));
}

PyObject* PyInput::take(PyInput *self, PyObject *Py_UNUSED(ignored))
{
    using dds::core::xtypes::DynamicData;
    using rti::routing::processor::LoanedSamples;

    PyObject *py_samples = NULL;
    try {
        rti::routing::processor::detail::NativeSamples native_samples;
        self->get()->take(
                self->get()->stream_reader_data,
                &native_samples.sample_array_,
                &native_samples.info_array_,
                &native_samples.length_,
                self->native_env_);
        RTI_ROUTING_THROW_ON_ENV_ERROR(self->native_env_);
        auto native_loaned_samples = LoanedSamples<native_data_type>(
                self->get(),
                native_samples,
                self->native_env_);
        py_samples = PyInput::sample_list(native_loaned_samples);
    } catch (const std::exception &ex) {
        PyErr_Format(PyExc_RuntimeError, "%s", ex.what());
        return NULL;
    }

    return py_samples;
}


static PyMethodDef PyInput_g_methods[] = {
    {
        "name",
        (PyCFunction) PyInput::name,
        METH_NOARGS,
        "Return the name, combining the first and last name"
    },
    {
        "take",
        (PyCFunction) PyInput::take,
        METH_NOARGS,
        "takes all available samples from the input's cache"
    },
    {NULL}  /* Sentinel */
};

static PyTypeObject PyInput_g_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "pyproc.Input",
    .tp_doc = "Input object",
    .tp_basicsize = sizeof(PyInput),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyNativeWrapper<PyInputType>::new_object,
    .tp_dealloc = PyNativeWrapper<PyInputType>::delete_object,
    .tp_methods = PyInput_g_methods
};


PyInput::PyInput(
        RTI_RoutingServiceStreamReaderExt* native,
        RTI_RoutingServiceRoute *native_route,
        RTI_RoutingServiceEnvironment *environment)
        : PyNativeWrapper(native),
        native_route_(native_route),
        native_env_(environment)
{

}


RTI_RoutingServiceRoute* PyInput::native_route()
{
    return native_route_;
}

PyObject* PyInput::sample_list(native_loaned_samples& loaned_samples)
{
    PyObject* py_list = PyList_New(loaned_samples.length());
    if (py_list == NULL) {
        PyErr_Print();
        throw dds::core::Error("PyInput::sample_list: error creating sample list");
    }

    // Convert samples into dictionaries
    for (int32_t i = 0; i < loaned_samples.length(); ++i) {
        if (PyList_SetItem(
                py_list,
                i,
                new PySample(loaned_samples[i])) != 0) {
            PyErr_Print();
            throw dds::core::Error("PyInput::sample_list: error creating sample item");
        }
    }

    return py_list;
}


PyTypeObject* PyInputType::type()
{
    return &PyInput_g_type;
}

const std::string& PyInputType::name()
{
    static std::string __name("Input");

    return __name;
}


} } }

