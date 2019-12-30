#include "Python.h"

#include <stdio.h>
#include <stdlib.h>
#include <iterator>
#include <dds/core/corefwd.hpp>
#include "PyInput.hpp"


namespace rti { namespace routing { namespace py {

const RTI_RoutingServiceSelectorState&
PySelector::DEFAULT_STATE()
{
    static RTI_RoutingServiceSelectorState init_state =
            RTI_RoutingServiceSelectorState_INITIALIZER;

    return init_state;
}

#define RTI_PY_CHECK_AND_THROW(TYPE, OBJECT, MEMBER) \
if(!Py ## TYPE ## _Check((OBJECT))) {\
    throw dds::core::InvalidArgumentError(\
            "element=" #MEMBER " is not a " #TYPE); \
}

void PySelector::build(
        RTI_RoutingServiceSelectorState& state,
        PyObject *py_dict)
{

    PyObject *py_item;

    //sample_state
    py_item = PyDict_GetItemString(py_dict, "sample_state");
    if (py_item != NULL) {
        RTI_PY_CHECK_AND_THROW(Long, py_item, sample_state);
        state.sample_state = (DDS_SampleStateMask) PyLong_AsLong(py_item);
    }

    // view_state
    py_item = PyDict_GetItemString(py_dict, "view_state");
    if (py_item != NULL) {
        RTI_PY_CHECK_AND_THROW(Long, py_item, view_state);
        state.view_state = (DDS_ViewStateKind) PyLong_AsLong(py_item);
    }

    // instance_state
    py_item = PyDict_GetItemString(py_dict, "instance_state");
    if (py_item != NULL) {
        RTI_PY_CHECK_AND_THROW(Long, py_item, instance_state);
        state.view_state = (DDS_ViewStateKind) PyLong_AsLong(py_item);
    }

    // instance
    py_item = PyDict_GetItemString(py_dict, "instance");
    if (py_item != NULL) {
        RTI_PY_CHECK_AND_THROW(Dict, py_item, instance);
        to_native(state.instance_handle, py_item);
        state.instance_selection = RTI_ROUTING_SERVICE_THIS_INSTANCE_SELECTION;
    }

    // next_instance
    py_item = PyDict_GetItemString(py_dict, "next_instance");
    if (py_item != NULL) {
        RTI_PY_CHECK_AND_THROW(Dict, py_item, instance);
        to_native(state.instance_handle, py_item);
        state.instance_selection = RTI_ROUTING_SERVICE_NEXT_INSTANCE_SELECTION;
    }

    // max_samples
    py_item = PyDict_GetItemString(py_dict, "max_samples");
    if (py_item != NULL) {
        RTI_PY_CHECK_AND_THROW(Long, py_item, max_samples);
        state.sample_count_max = (int32_t) PyLong_AsLong(py_item);
    }
}


/*
 * --- PyInput Python methods -------------------------------------------------
 */
PyObject* PyInput::info(PyInput* self, void* closure)
{
    Py_INCREF(self->info_.get());
    return self->info_.get();
}


PyObject* PyInput::take(PyInput *self, PyObject *args)
{
    return PyInput::read_or_take_w_selector(
            self,
            args,
            self->get()->take_w_selector);
}

PyObject* PyInput::read(PyInput* self, PyObject* args)
{
    return PyInput::read_or_take_w_selector(
            self,
            args,
            self->get()->read_w_selector);
}

static PyGetSetDef PyInput_g_getsetters[] = {
    {
        (char *) "info",
        (getter) PyInput::info,
        (setter) NULL,
        (char *) "information properties of this input",
        NULL
    },
    {NULL}  /* Sentinel */
};

static PyMethodDef PyInput_g_methods[] = {
    {
        "take",
        (PyCFunction) PyInput::take,
        METH_VARARGS,
        "takes all available samples from the input's cache"
    },
    {
        "read",
        (PyCFunction) PyInput::read,
        METH_VARARGS,
        "reads all available samples from the input's cache"
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
    .tp_dealloc = PyNativeWrapper<PyInputType>::delete_object,
    .tp_methods = PyInput_g_methods,
    .tp_getset = PyInput_g_getsetters
};


PyInput::PyInput(
        RTI_RoutingServiceStreamReaderExt* native,
        RTI_RoutingServiceRoute *native_route,
        RTI_RoutingServiceEnvironment *environment)
        : PyNativeWrapper(native),
        native_route_(native_route),
        native_env_(environment),
        info_(PyDict_New())
{
    build_info();
}



RTI_RoutingServiceRoute* PyInput::native_route()
{
    return native_route_;
}

PyObject* PyInput::sample_list(
            native_loaned_samples& loaned_samples,
            bool has_infos)
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
                new PySample(loaned_samples[i], has_infos)) != 0) {
            PyErr_Print();
            throw dds::core::Error("PyInput::sample_list: error creating sample item");
        }
    }

    return py_list;
}

PyObject* PyInput::read_or_take_w_selector(
        PyInput *self,
        PyObject *args,
        RTI_RoutingServiceStreamReaderExt_TakeWithSelectorFcn read_or_take_w_selector)
{
    using dds::core::xtypes::DynamicData;
    using rti::routing::processor::LoanedSamples;

    PyObject *py_dict = NULL;
    if (!PyArg_ParseTuple(args, "|O!", &PyDict_Type, &py_dict)) {
        return NULL;
    }

    PyObject *py_samples = NULL;
    try {
        rti::routing::processor::detail::NativeSamples native_samples;

        if (py_dict == NULL) {
            read_or_take_w_selector(
                    self->get()->stream_reader_data,
                    &native_samples.sample_array_,
                    &native_samples.info_array_,
                    &native_samples.length_,
                    &(PySelector::DEFAULT_STATE()),
                    self->native_env_);
        } else {
            RTI_RoutingServiceSelectorState selector_state =
                    PySelector::DEFAULT_STATE();

            PySelector::build(selector_state, py_dict);

            read_or_take_w_selector(
                    self->get()->stream_reader_data,
                    &native_samples.sample_array_,
                    &native_samples.info_array_,
                    &native_samples.length_,
                    &selector_state,
                    self->native_env_);
        }
        RTI_ROUTING_THROW_ON_ENV_ERROR(self->native_env_);
        auto native_loaned_samples = LoanedSamples<native_data_type>(
                self->get(),
                native_samples,
                self->native_env_);
        py_samples = PyInput::sample_list(
                native_loaned_samples,
                (native_samples.info_array_ != NULL));
    } catch (const std::exception &ex) {
        PyErr_Format(PyExc_RuntimeError, "%s", ex.what());
        return NULL;
    }

    return py_samples;
}

void PyInput::build_info()
{
    const char *name =
            RTI_RoutingServiceRoute_get_input_name(native_route(), get());
    RTI_PY_ADD_DICT_ITEM_VALUE(info_.get(), name, PyUnicode_FromString);


    const RTI_RoutingServiceStreamInfo& stream_info =
            *RTI_RoutingServiceRoute_get_input_stream_info(
                native_route(),
                get());
     RTI_PY_ADD_DICT_ITEM_VALUE(info_.get(), stream_info, from_native);
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
