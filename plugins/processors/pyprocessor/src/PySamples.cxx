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

PySample::PySample(
        const native_sample& loaned_sample)
        :data_(DynamicDataConverter::to_dictionary(loaned_sample.data())),
        info_(SampleInfoConverter::to_dictionary(loaned_sample.info()))
{
}

void * PySample::operator new(size_t size)
{
    return PySampleType::type()->tp_alloc(
            PySampleType::type(),
            size);
}

void PySample::operator delete(void* object)
{
    Py_TYPE(object)->tp_free((PyObject *) object);
}

static PyGetSetDef PySample_g_getsetters[] = {
    {
        (char *) "data",
        (getter) PySample::data,
        (setter) NULL,
        (char *) "data portion of the sample name",
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

static PyTypeObject PySample_g_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "pyproc.Sample",
    .tp_doc = "Loaned Sample object",
    .tp_basicsize = sizeof(PySample),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = NULL,
    .tp_methods = PySample_g_methods,
    .tp_getset = PySample_g_getsetters
};

PyTypeObject* PySampleType::type()
{
    return &PySample_g_type;
}

const std::string& PySampleType::name()
{
    static std::string __name("Sample");

    return __name;
}



} } }

