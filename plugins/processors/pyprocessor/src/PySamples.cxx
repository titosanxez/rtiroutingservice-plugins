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
    if (self->info_ != NULL) {
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
        const native_data* data, const native_info *info)
        :data_(build_data(data, info)),
        info_((info != NULL) ? from_native(info->extensions().native()) : NULL),
        valid_(PyBool_FromLong(info->valid()))
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
        _sample_type.tp_name = "proc.Sample";
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



} } }

