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

#include "PyLoanedSamples.hpp"

namespace rti { namespace routing { namespace py {

/*
 * --- PyInfo Python methods --------------------------------------------------
 */
/*
 * --- PyInfo class implementation --------------------------------------------
 */

PyInfo::PyInfo(const PyInfoType::native_type* native_data)
        :PyNativeWrapper(const_cast<PyInfoType::native_type*>(native_data))
{

}


static PyTypeObject PyInfo_g_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "pyproc.SampleInfo",
    .tp_doc = "Loaned SampleInfo object",
    .tp_basicsize = sizeof(PyInfo),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = NULL
};

PyTypeObject *PyInfoType::type()
{
    return &PyInfo_g_type;
}


/*
 * --- PyData Python methods --------------------------------------------------
 */
/*
 * --- PyData class implementation --------------------------------------------
 */

PyData::PyData(const PyDataType::native_type* native_data)
        :PyNativeWrapper(const_cast<PyDataType::native_type*>(native_data))
{

}


static PyTypeObject PyData_g_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "pyproc.SampleData",
    .tp_doc = "Loaned SampleData object",
    .tp_basicsize = sizeof(PyData),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = NULL
};

PyTypeObject *PyDataType::type()
{
    return &PyData_g_type;
}

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
        :data_(new PyData(&(loaned_sample.data()))),
        info_(new PyInfo(&(loaned_sample.info())))
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

/*
 * --- PyLoanedSamples Python methods -----------------------------------------
 */

PyObject* PyLoanedSamples::get_item(
        PyLoanedSamples *self,
        Py_ssize_t index)
{
    using dds::core::xtypes::DynamicData;
    using rti::routing::processor::LoanedSample;

    try {
        return new PySample(self->get()[index]);
    } catch (const std::exception &ex) {
        PyErr_Format(PyExc_RuntimeError, "%s", ex.what());
        return NULL;
    }
}

PyObject* PyLoanedSamples::length(PyLoanedSamples* self, void* closure)
{
    return PyLong_FromLong(self->get().length());
}

static
void PyLoanedSamples_dealloc(PyLoanedSamples *self)
{
    delete self;
}

static PyGetSetDef PyLoanedSamples_g_getsetters[] = {
    {
        (char *) "length",
        (getter) PyLoanedSamples::length,
        (setter) NULL,
        (char *) "total number of read samples",
        NULL
    },
    {NULL}  /* Sentinel */
};

static PyMethodDef PyLoanedSamples_g_methods[] = {
//    {
//        "__getitem__",
//        (PyCFunction) PyLoanedSamples_get_item,
//        METH_VARARGS,
//        "returns the Sample at the specified index"
//    },
    {NULL}  /* Sentinel */
};

static PySequenceMethods PyLaonedSamples_g_sequence = {
    .sq_item = (ssizeargfunc) PyLoanedSamples::get_item
};

static PyTypeObject PyLoanedSamples_g_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "pyproc.LoanedSamples",
    .tp_doc = "LoanedSamples object",
    .tp_basicsize = sizeof(PyLoanedSamples),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = NULL,
    .tp_dealloc = (destructor) PyLoanedSamples_dealloc,
    .tp_methods = PyLoanedSamples_g_methods,
    .tp_as_sequence = &PyLaonedSamples_g_sequence,
    .tp_getset = PyLoanedSamples_g_getsetters,
};

/*
 * --- PyLoanedSamples class implementation -----------------------------------
 */

void * PyLoanedSamples::operator new(size_t size)
{
    return PyLoanedSamples::type()->tp_alloc(
            PyLoanedSamples::type(),
            size);
}

void PyLoanedSamples::operator delete(void* object)
{
    Py_TYPE(object)->tp_free((PyObject *) object);
}


PyLoanedSamples::PyLoanedSamples(Native& loaned_samples)
    : loaned_samples_(std::move(loaned_samples))
{
    PyObject_Init(this, PyLoanedSamples::type());
}

PyLoanedSamples::Native& PyLoanedSamples::get()
{
    return loaned_samples_;
}

PyTypeObject* PyLoanedSamples::type()
{
    return &PyLoanedSamples_g_type;
}



} } }

