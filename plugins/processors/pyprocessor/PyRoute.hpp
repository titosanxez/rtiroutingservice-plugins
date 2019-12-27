

#ifndef PYROUTE_HPP
#define PYROUTE_HPP

#include "Python.h"

#include <rti/routing/processor/Processor.hpp>
#include "NativeUtils.h"
#include "PyInput.hpp"


namespace rti { namespace routing { namespace py {

class PyRouteType
{
public:
    typedef RTI_RoutingServiceRoute native_type;
    static PyTypeObject* type();
};

class PyRoute : public PyNativeWrapper<PyRouteType>
{
public:
    PyRoute(RTI_RoutingServiceRoute *native);
    PyInput* input(RTI_RoutingServiceStreamReaderExt *native_input);
    PyInput* input(int32_t index);
    static PyTypeObject* type();
};

}}}

#endif /* PYROUTE_HPP */

