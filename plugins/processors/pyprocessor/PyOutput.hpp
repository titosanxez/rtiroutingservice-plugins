/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   PyOutput.hpp
 * Author: asanchez
 *
 * Created on December 26, 2019, 9:59 PM
 */

#ifndef PYOUTPUT_HPP
    #define PYOUTPUT_HPP

#include "Python.h"
#include <dds/core/xtypes/DynamicData.hpp>
#include <rti/routing/processor/Processor.hpp>
#include "NativeUtils.h"
#include "PyLoanedSamples.hpp"

namespace rti { namespace routing { namespace py {

class PyOutputType
{
public:
    typedef RTI_RoutingServiceStreamWriterExt native_type;
    static PyTypeObject* type();
};

class PyOutput : public PyNativeWrapper<PyOutputType>
{
public:
    typedef RTI_RoutingServiceStreamWriterExt native_type;
    typedef typename PyDataType::native_type native_data_type;

    PyOutput(
            RTI_RoutingServiceStreamWriterExt *native,
            RTI_RoutingServiceRoute *native_route,
            RTI_RoutingServiceEnvironment *environment);
    RTI_RoutingServiceRoute *native_route();

    static PyObject* name(PyOutput *self, PyObject *Py_UNUSED(ignored));
    static PyObject* write(PyOutput *self, PyObject *args);

private:
    RTI_RoutingServiceRoute *native_route_;
    RTI_RoutingServiceEnvironment *native_env_;
};

} } }

#endif /* OUTPUT_HPP */

