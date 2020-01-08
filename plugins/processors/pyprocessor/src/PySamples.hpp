/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   LoanedSamples.hpp
 * Author: asanchez
 *
 * Created on December 26, 2019, 9:49 AM
 */

#ifndef LOANEDSAMPLES_HPP
#define LOANEDSAMPLES_HPP

#include "Python.h"

#include <dds/core/corefwd.hpp>
#include <dds/core/xtypes/DynamicData.hpp>
#include <rti/routing/processor/LoanedSamples.hpp>

#include "NativeUtils.hpp"
#include "PyDynamicData.hpp"

namespace rti { namespace routing { namespace py {

class PyOutput;

class PySampleType : public PyObject {
public:
    typedef dds::sub::SampleInfo native_type;
    static PyTypeObject* type();
    static const std::string& name();
};

class PySample : public PyAllocatorGeneric<PySampleType, PySample> {
public:
    typedef dds::core::xtypes::DynamicData native_data;
    typedef dds::sub::SampleInfo native_info;
    PySample(const native_data* data, const native_info *info);
    ~PySample();

    /* python methods */
    static PyObject* valid_data(PySample *self, void* closure);
    static PyObject* data(PySample *self, void* closure);
    static PyObject* info(PySample *self, void* closure);
    static Py_ssize_t count(PySample *self);
    static PyObject* binary(PySample *self, PyObject *key);
    static int set_item(PySample *, PyObject *, PyObject *);


    static PyObject * print(PySample *self);
    static PyObject * representation(PySample *self);

private:
    static PyObject* build_data(
            const native_data* data,
            const native_info *info);
private:
    friend class PyOutput;
    PyObject* data_; //as dictionary
    PyObject *info_; //as dictionary
    PyObject *valid_;
};

} } }

#endif /* LOANEDSAMPLES_HPP */

