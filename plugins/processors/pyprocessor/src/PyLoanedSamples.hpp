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
#include "NativeUtils.h"

namespace rti { namespace routing { namespace py {

class PyDataType {
public:
    typedef dds::core::xtypes::DynamicData native_type;
    static PyTypeObject* type();
    static const std::string& name();
};

class PyData : public PyNativeWrapper<PyDataType>
{
public:
    typedef typename PyDataType::native_type native_type;
    PyData(const PyDataType::native_type* native_data);
};

class PyInfoType {
public:
    typedef dds::sub::SampleInfo native_type;
    static PyTypeObject* type();
    static const std::string& name();
};

class PyInfo : public PyNativeWrapper<PyInfoType> {
public:
    typedef typename PyInfoType::native_type native_type;
    PyInfo(const PyInfoType::native_type* native_data);
};


class PySampleType : public PyObject {
public:
    typedef dds::sub::SampleInfo native_type;
    static PyTypeObject* type();
    static const std::string& name();
};

class PySample : public PyObject {
public:
    typedef rti::routing::processor::LoanedSample<
            PyData::native_type,
            PyInfo::native_type> native_sample;
    PySample(const native_sample& sample);
    void* operator new(size_t size);
    void operator delete(void* object);

    /* python methods */
    static PyObject* data(PySample *self, void* closure);
    static PyObject* info(PySample *self, void* closure);

private:
    PyData *data_;
    PyInfo *info_;
};

class PyLoanedSamplesType {
public:
    typedef dds::core::xtypes::DynamicData native_type;
    static PyTypeObject* type();
    static const std::string& name();
};

class PyLoanedSamples : public PyObject {
public:
    typedef dds::core::xtypes::DynamicData DynamicData;
    typedef rti::routing::processor::LoanedSamples<DynamicData> Native;

    PyLoanedSamples(Native& loaned_samples);

    void* operator new(size_t size);

    void operator delete(void* object);

    Native& get();
    
    /* python methods */
    static PyObject* get_item(
            PyLoanedSamples *self,
            Py_ssize_t index);

    static PyObject* length(PyLoanedSamples *self, void* closure);

private:
    Native loaned_samples_;
};

PyTypeObject* PyLoanedSamples_type();

} } }

#endif /* LOANEDSAMPLES_HPP */

