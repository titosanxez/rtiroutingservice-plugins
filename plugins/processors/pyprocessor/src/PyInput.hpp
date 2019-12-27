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
#include "NativeUtils.h"
#include "PyLoanedSamples.hpp"

namespace rti { namespace routing { namespace py {

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
    typedef typename PyDataType::native_type native_data_type;

    PyInput(
            RTI_RoutingServiceStreamReaderExt *native,
            RTI_RoutingServiceRoute *native_route,
            RTI_RoutingServiceEnvironment *environment);
    RTI_RoutingServiceRoute *native_route();

    static PyObject* name(PyInput *self, PyObject *Py_UNUSED(ignored));
    static PyObject* take(PyInput *self, PyObject *Py_UNUSED(ignored));

private:
    RTI_RoutingServiceRoute *native_route_;
    RTI_RoutingServiceEnvironment *native_env_;
};


} } }

#endif /* PYINPUT_HPP */

