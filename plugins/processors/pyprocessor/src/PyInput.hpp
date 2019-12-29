/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   PyInput.hpp
 * Author: asanchez
 *
 * Created on December 25, 2019, 7:24 PM
 */

#ifndef PYINPUT_HPP
#define PYINPUT_HPP


#include "Python.h"
#include <dds/core/xtypes/DynamicData.hpp>
#include <rti/routing/processor/Processor.hpp>
#include "NativeUtils.hpp"
#include "PySamples.hpp"

namespace rti { namespace routing { namespace py {

class PySelector
{
public:
    static void build(
            RTI_RoutingServiceSelectorState& state,
            PyObject *py_dict);

    const static RTI_RoutingServiceSelectorState& DEFAULT_STATE();
};

class PyInputType
{
public:
    typedef RTI_RoutingServiceStreamReaderExt native_type;
    static PyTypeObject* type();
    static const std::string& name();
};

class PyInput : public PyNativeWrapper<PyInputType>
{
public:
    typedef RTI_RoutingServiceStreamReaderExt native_type;
    typedef dds::core::xtypes::DynamicData native_data_type;
    typedef rti::routing::processor::LoanedSamples<native_data_type> native_loaned_samples;

    PyInput(
            RTI_RoutingServiceStreamReaderExt *native,
            RTI_RoutingServiceRoute *native_route,
            RTI_RoutingServiceEnvironment *environment);
    RTI_RoutingServiceRoute *native_route();

    static PyObject* name(PyInput *self, void *closure);
    static PyObject* stream_name(PyInput *self,void *closure );
    static PyObject* take(PyInput *self, PyObject *arg);
    static PyObject* read(PyInput *self, PyObject *arg);

private:
    static PyObject* sample_list(
            native_loaned_samples& loaned_samples,
            bool has_infos);
    static PyObject* read_or_take_w_selector(
            PyInput *self,
            PyObject *args,
            RTI_RoutingServiceStreamReaderExt_TakeWithSelectorFcn read_or_take);


private:
    friend class PySelector;
    RTI_RoutingServiceRoute *native_route_;
    RTI_RoutingServiceEnvironment *native_env_;
};


} } }

#endif /* PYINPUT_HPP */

