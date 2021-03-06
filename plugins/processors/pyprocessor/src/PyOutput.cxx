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
PyObject* PyOutput::info(PyOutput* self, void* closure)
{
    Py_INCREF(self->info_.get());
    return self->info_.get();
}


PyObject* PyOutput::write(PyOutput *self, PyObject *args)
{
    static const RTI_RoutingServiceSample out_data =
            reinterpret_cast<void **>(&self->output_data_);
    static const RTI_RoutingServiceSample *out_data_list = &out_data;
    static const RTI_RoutingServiceSampleInfo out_info =
            reinterpret_cast<void **>(&self->output_info_);
    const RTI_RoutingServiceSampleInfo *out_info_list = NULL;

    PyObject *py_sample = NULL;
    PyObject *py_data = NULL;
    PyObject *py_info = NULL;
    if (!PyArg_ParseTuple(
            args,
            "O",
            &py_sample)) {
        return NULL;
    }

    if (py_sample->ob_type == PySampleType::type()) {
        py_data = ((PySample *) py_sample)->data_;
        py_info = ((PySample *) py_sample)->info_;
    } else if (PyDict_Check(py_sample)) {
        py_data = py_sample;
    } else {
        PyErr_SetString(
                PyExc_ValueError,
                "sample parameter must be a Sample or data dictionary");
        return NULL;
    }

    try {
        DynamicDataConverter::to_dynamic_data(
                self->output_data_,
                py_data);
        if (py_info != NULL) {
            to_native(self->output_info_->native(), py_info);
            out_info_list = &out_info;
        }

        self->get()->write(
                self->get()->stream_writer_data,
                out_data_list,
                out_info_list,
                1,
                self->native_env_);
        RTI_ROUTING_THROW_ON_ENV_ERROR(self->native_env_);
    } catch (const std::exception &ex) {
        PyErr_Format(PyExc_RuntimeError, "%s", ex.what());
        return NULL;
    }

    Py_RETURN_NONE;
}

static PyGetSetDef PyOutput_g_getsetters[] = {
    {
        (char *) "info",
        (getter) PyOutput::info,
        (setter) NULL,
        (char *) "information properties of this output",
        NULL
    },
    {NULL}  /* Sentinel */
};

static PyMethodDef PyOutput_g_methods[] = {
    {
        "write",
        (PyCFunction) PyOutput::write,
        METH_VARARGS,
        "write a sample by its data and info portion"
    },
    {NULL}  /* Sentinel */
};

const dds::core::xtypes::DynamicType& dynamic_type(
        RTI_RoutingServiceStreamWriterExt* native_output,
        RTI_RoutingServiceRoute *native_route)
{
    const RTI_RoutingServiceStreamInfo *stream_info =
            RTI_RoutingServiceRoute_get_output_stream_info(
                    native_route,
                    native_output);
    dds::core::xtypes::DynamicType *type_code =
            static_cast<dds::core::xtypes::DynamicType *> (
            stream_info->type_info.type_representation);
    return *type_code;
}


PyOutput::PyOutput(
        RTI_RoutingServiceStreamWriterExt* native,
        int32_t index,
        RTI_RoutingServiceRoute *native_route,
        RTI_RoutingServiceEnvironment *environment)
        : PyNativeWrapper(native),
        index_(index),
        native_route_(native_route),
        native_env_(environment),
        info_(PyDict_New()),
        output_data_(dynamic_type(native, native_route))
{
    build_info();
}

RTI_RoutingServiceRoute* PyOutput::native_route()
{
    return native_route_;
}

void PyOutput::build_info()
{
    int32_t index = this->index_;
    RTI_PY_ADD_DICT_ITEM_VALUE(info_.get(), index, PyLong_FromLong);

    const char *name =
            RTI_RoutingServiceRoute_get_output_name(native_route(), get());
    RTI_PY_ADD_DICT_ITEM_VALUE(info_.get(), name, PyUnicode_FromString);


    const RTI_RoutingServiceStreamInfo& stream_info =
            *RTI_RoutingServiceRoute_get_output_stream_info(
                native_route(),
                get());
     RTI_PY_ADD_DICT_ITEM_VALUE(info_.get(), stream_info, from_native);
}


PyTypeObject* PyOutputType::type()
{
    static PyTypeObject _output_type;
    static bool _init = false;

    if (!_init) {
        RTIOsapiMemory_zero(&_output_type, sizeof (_output_type));
        _output_type.tp_name = "rti.routing.proc.Output";
        _output_type.tp_doc = "Output object";
        _output_type.tp_basicsize = sizeof (PyOutput);
        _output_type.tp_itemsize = 0;
        _output_type.tp_flags = Py_TPFLAGS_DEFAULT;
        _output_type.tp_dealloc = PyNativeWrapper<PyOutputType, PyOutput>::delete_object;
        _output_type.tp_methods = PyOutput_g_methods;
        _output_type.tp_getset = PyOutput_g_getsetters;
        _init = true;
    }

    return &_output_type;
}

const std::string& PyOutputType::name()
{
    static std::string __name("Output");

    return __name;
}



} } }

