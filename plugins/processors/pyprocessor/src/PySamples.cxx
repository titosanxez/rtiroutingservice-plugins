/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   LoanedSamples.cpp
 * Author: asanchez
 *
 * Created on December 26, 2019, 9:49 AM
 */

#include "PySamples.hpp"

namespace rti { namespace routing { namespace py {

/*
 * --- PySample Python methods ------------------------------------------------
 */

PyObject* PySample::data(PySample* self, void*)
{
    if (self->data_ == NULL) {
        try {
            self->data_ = build_data(self->native_data_, self->native_info_);
        } catch (const std::exception &ex) {
            PyErr_Format(PyExc_RuntimeError, "%s", ex.what());
            return NULL;
        }
    }
    Py_INCREF(self->data_);
    return self->data_;
}

PyObject* PySample::valid_data(PySample* self, void*)
{
    Py_INCREF(self->valid_);
    return self->valid_;
}

PyObject* PySample::info(PySample* self, void*)
{
   if (self->info_ == NULL && self->native_info_ != NULL) {
        try {
            self->info_ = from_native(self->native_info_->extensions().native());
        } catch (const std::exception &ex) {
            PyErr_Format(PyExc_RuntimeError, "%s", ex.what());
            return NULL;
        }
        Py_INCREF(self->info_);
        return self->info_;
    }

    return Py_None;
}
/*
 * --- PySample class implementation ------------------------------------------
 */


PyObject* PySample::build_data(
        const native_data* data,
        const native_info* info)
{
    if (info != NULL && !info->valid()) {
        /* empty data */
        return PyDict_New();
    }

    return DynamicDataConverter::from_dynamic_data(*data);
}


PySample::PySample(
        const native_data* native_data, const native_info *native_info)
        :data_(NULL),
        info_(NULL),
        valid_(PyBool_FromLong(native_info->valid())),
        native_data_(native_data),
        native_info_(native_info)
{
}

PySample::~PySample()
{
    if (data_ != NULL) {
        Py_DECREF(data_);
    }
    if (info_ != NULL) {
        Py_DECREF(info_);
    }
    if (valid_ != NULL) {
        Py_DECREF(valid_);
    }
}

Py_ssize_t PySample::count(PySample *self)
{
    return PyDict_Size(self->data_);
}

PyObject* PySample::binary(PySample *self, PyObject *key)
{
    if (!PyUnicode_Check(key)) {
        PyErr_SetString(
                PyExc_ValueError,
                "PySample::binary: key must be an string represent a member name");
        return NULL;
    }

    return PyDict_GetItem(self->data_, key);
}

int PySample::set_item(PySample* self, PyObject *key , PyObject *value)
{
    if (value == NULL) {
        return PyObject_DelItem(self->data_, key);
    }

    return PyObject_SetItem(self->data_, key, value);
}


PyObject* PySample::print(PySample* self)
{
    return PyDict_Type.tp_str(self->data_);
}

PyObject* PySample::representation(PySample* self)
{
    return PyDict_Type.tp_repr(self->data_);
}



static PyMappingMethods PySample_g_mapping = {
    .mp_length = (lenfunc) PySample::binary,
    .mp_subscript = (binaryfunc) PySample::binary,
    .mp_ass_subscript = (objobjargproc) PySample::set_item
};

static PyGetSetDef PySample_g_getsetters[] = {
    {
        (char *) "valid_data",
        (getter) PySample::valid_data,
        (setter) NULL,
        (char *) "returns whether the data portion is valid or not",
        NULL
    },
        {
        (char *) "data",
        (getter) PySample::data,
        (setter) NULL,
        (char *) "returns whether the data portion is valid or not",
        NULL
    },
    {
        (char *) "info",
        (getter) PySample::info,
        (setter) NULL,
        (char *) "info portion of the sample name",
        NULL
    },
    {NULL}  /* Sentinel */
};

static PyMethodDef PySample_g_methods[] = {
    {NULL}  /* Sentinel */
};



PyTypeObject* PySampleType::type()
{
    static PyTypeObject _sample_type;
    static bool _init = false;

    if (!_init) {
        RTIOsapiMemory_zero(&_sample_type, sizeof (_sample_type));
        _sample_type.tp_name = "rti.routing.proc.Sample";
        _sample_type.tp_basicsize = sizeof (PySample);
        _sample_type.tp_itemsize = 0;
        _sample_type.tp_dealloc = PyAllocatorGeneric<PySampleType, PySample>::delete_object;
        _sample_type.tp_repr = (reprfunc) PySample::representation;
        _sample_type.tp_as_mapping = &PySample_g_mapping;
        _sample_type.tp_str = (reprfunc) PySample::print;
        _sample_type.tp_flags = Py_TPFLAGS_DEFAULT;
        _sample_type.tp_doc = "Sample object";
        _sample_type.tp_methods = PySample_g_methods;
        _sample_type.tp_getset = PySample_g_getsetters;
        _sample_type.tp_new = NULL;
        _init = true;
    }

    return &_sample_type;
}

const std::string& PySampleType::name()
{
    static std::string __name("Sample");

    return __name;
}

/*
 * --- PyLoanedSamples --------------------------------------------------------
 */

PyLoanedSamples::PyLoanedSamples(
        native_loaned_samples& native_samples,
        RTI_RoutingServiceStreamReaderExt *native_input,
        RTI_RoutingServiceEnvironment *native_env)
        :py_list_(PyList_New(native_samples.length_)),
        native_samples_(native_samples),
        native_input_(native_input),
        native_env_(native_env)
{
    // Convert samples into dictionaries
    for (int32_t i = 0; i < native_samples.length_; ++i) {
        PyList_SET_ITEM(
                py_list_,
                i,
                new PySample(
                    static_cast<const PySample::native_data*> (native_samples.sample_array_[i]),
                    static_cast<const PySample::native_info*> (native_samples.info_array_[i])));
    }
}

PyLoanedSamples::~PyLoanedSamples()
{
    native_input_->return_loan(
            native_input_->stream_reader_data,
            native_samples_.sample_array_,
            native_samples_.info_array_,
            native_samples_.length_,
            native_env_);
    if (RTI_RoutingServiceEnvironment_error_occurred(native_env_)) {
        PyErr_Format(
                PyExc_RuntimeError,
                "%s",
                RTI_RoutingServiceEnvironment_get_error_message(native_env_));
    }
}


PyObject* PyLoanedSamples::binary(PyLoanedSamples* self, PyObject* key)
{
    return PyObject_GetItem(self->py_list_, key);
}

Py_ssize_t PyLoanedSamples::count(PyLoanedSamples* self)
{
    return PyList_GET_SIZE(self->py_list_);
}

PyObject* PyLoanedSamples::get_iterator(PyLoanedSamples* self)
{
    return PyObject_GetIter(self->py_list_);
}

PyObject* PyLoanedSamples::iterator_next(PyObject* self)
{
    return PyIter_Next(self);
}

static PyMappingMethods PyLoanedSamples_g_mapping = {
    .mp_length = (lenfunc) PyLoanedSamples::binary,
    .mp_subscript = (binaryfunc) PyLoanedSamples::binary,
    .mp_ass_subscript = NULL
};

PyTypeObject* PyLoanedSamplesType::type()
{
     static PyTypeObject __loaned_samples_type;
    static bool _init = false;

    if (!_init) {
        RTIOsapiMemory_zero(&__loaned_samples_type, sizeof (__loaned_samples_type));
        __loaned_samples_type.tp_name = "rti.routing.proc.LoanedSamples";
        __loaned_samples_type.tp_basicsize = sizeof (PyLoanedSamples);
        __loaned_samples_type.tp_itemsize = 0;
        __loaned_samples_type.tp_dealloc =
                PyAllocatorGeneric<PyLoanedSamplesType, PyLoanedSamples>::delete_object;
        __loaned_samples_type.tp_as_mapping = &PyLoanedSamples_g_mapping;
        __loaned_samples_type.tp_iter = (getiterfunc) PyLoanedSamples::get_iterator;
        __loaned_samples_type.tp_iternext = (iternextfunc) PyLoanedSamples::iterator_next;
        __loaned_samples_type.tp_flags = Py_TPFLAGS_DEFAULT;
        __loaned_samples_type.tp_doc = "LoanedSamples object";
        __loaned_samples_type.tp_new = NULL;
        _init = true;
    }

    return &__loaned_samples_type;
}

const std::string& PyLoanedSamplesType::name()
{
    static std::string __name("LoanedSamples");

    return __name;
}




} } }

