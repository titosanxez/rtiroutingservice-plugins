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

class PyInput;

class PySelectorBuilder
{
public:
    const static RTI_RoutingServiceSelectorState& DEFAULT_STATE();


    PySelectorBuilder(PyInput *input, PyObject *py_dict);
    ~PySelectorBuilder();

private:
    void build(PyObject *py_dict);
    void create_content_query();
private:
    friend class PyInput;
    PyInput *input;
    RTI_RoutingServiceSelectorState state;
};

class PyInputType
{
public:
    typedef RTI_RoutingServiceStreamReaderExt native_type;
    static PyTypeObject* type();
    static const std::string& name();
};

class PyInput : public PyNativeWrapper<PyInputType, PyInput>
{
public:
    typedef RTI_RoutingServiceStreamReaderExt native_type;
    typedef dds::core::xtypes::DynamicData native_data;
     typedef dds::sub::SampleInfo native_info;
    typedef rti::routing::processor::detail::NativeSamples native_samples;

    PyInput(
            RTI_RoutingServiceStreamReaderExt *native,
            int32_t index,
            RTI_RoutingServiceRoute *native_route,
            RTI_RoutingServiceEnvironment *environment);

    RTI_RoutingServiceRoute *native_route();

    const char *name();
    static PyObject* info(PyInput *self,void *closure );
    static PyObject* take(PyInput *self, PyObject *arg);
    static PyObject* read(PyInput *self, PyObject *arg);

private:
    static PyObject* read_or_take_w_selector(
            PyInput *self,
            PyObject *args,
            RTI_RoutingServiceStreamReaderExt_TakeWithSelectorFcn read_or_take);
    void build_info();


private:
    friend class PySelectorBuilder;
    int32_t index_;
    RTI_RoutingServiceRoute *native_route_;
    RTI_RoutingServiceEnvironment *native_env_;
    PyObjectGuard info_;
};


} } }

#endif /* PYINPUT_HPP */

