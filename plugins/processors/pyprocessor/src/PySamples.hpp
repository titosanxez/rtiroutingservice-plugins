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


class PySampleType : public PyObject {
public:
    typedef dds::sub::SampleInfo native_type;
    static PyTypeObject* type();
    static const std::string& name();
};

class PySample : public PyObject {
public:
    typedef rti::routing::processor::LoanedSample<
            dds::core::xtypes::DynamicData,
            dds::sub::SampleInfo> native_sample;
    PySample(const native_sample& sample);
    void* operator new(size_t size);
    void operator delete(void* object);

    /* python methods */
    static PyObject* data(PySample *self, void* closure);
    static PyObject* info(PySample *self, void* closure);

private:
    PyObject* data_; //as dictionary
    PyObject *info_; //as dictionary
};

} } }

#endif /* LOANEDSAMPLES_HPP */

