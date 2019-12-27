/*
 * (c) 2018 Copyright, Real-Time Innovations, Inc.  All rights reserved.
 *
 * RTI grants Licensee a license to use, modify, compile, and create derivative
 * works of the Software.  Licensee has the right to distribute object form
 * only for use with RTI products.  The Software is provided "as is", with no
 * warranty of any type, including any warranty for fitness for any purpose.
 * RTI is under no obligation to maintain or support the Software.  RTI shall
 * not be liable for any incidental or consequential damages arising out of the
 * use or inability to use the software.
 */
#include <stdio.h>
#include <stdlib.h>
#include <iterator>

#include "Python.h"
#include <dds/core/corefwd.hpp>

#include <rti/routing/processor/ProcessorPlugin.hpp>
#include <rti/routing/processor/Processor.hpp>
#include <dds/core/xtypes/DynamicData.hpp>
#include <dds/core/xtypes/StructType.hpp>

#include "NativeUtils.h"
#include "PyProcessor.hpp"
#include "PyInput.hpp"
#include "PyLoanedSamples.hpp"

using namespace rti::routing;
using namespace rti::routing::processor;
using namespace rti::routing::adapter;
using namespace dds::core::xtypes;
using namespace dds::sub::status;


namespace rti { namespace routing { namespace py {

/*
 * --- PyProcessor -------------------------------------------------
 */

PyProcessor::PyProcessor(
        PyObject *py_processor,
        PyRoute *py_route)
        : py_processor_(py_processor),
          py_route_(py_route)
{
    RTIOsapiMemory_zero(&native_, sizeof(native_));
    /* initialize native implementation */
    native_.processor_data =
            static_cast<void*> (this);
    native_.on_route_event =
            PyProcessor::forward_on_route_event;
    Py_INCREF(py_processor);
}

PyProcessor::~PyProcessor()
{
    Py_DECREF(py_processor_);
}

RTI_RoutingServiceProcessor *
PyProcessor::create_native(
        PyProcessorPlugin *plugin,
        RTI_RoutingServiceRoute *native_route,
        const struct RTI_RoutingServiceProperties *native_properties,
        RTI_RoutingServiceEnvironment *environment)
{
    PyProcessor *forwarder;
    try {
        PyRoute *py_route = new PyRoute(native_route);
        PyObjectGuard py_properties = from_native(native_properties);

        forwarder = new PyProcessor(
                plugin->create_processor(py_route, py_properties.get()),
                py_route);
    } catch (const std::exception& ex) {
        RTI_RoutingServiceEnvironment_set_error(
                environment,
                "%s",
                ex.what());
    } catch (...) {
        RTI_RoutingServiceEnvironment_set_error(
                environment,
                "unexpected exception");
    }

    return (forwarder != NULL) ? forwarder->native() : NULL;
}

void PyProcessor::delete_native(
        PyProcessorPlugin *,
        RTI_RoutingServiceProcessor *native_processor,
        RTI_RoutingServiceEnvironment *environment)
{
    PyProcessor *processor_forwarder =
            static_cast<PyProcessor*> (native_processor->processor_data);
    try {
        delete processor_forwarder;
    } catch (const std::exception& ex) {
        RTI_RoutingServiceEnvironment_set_error(
                environment,
                "%s",
                ex.what());
    } catch (...) {
        RTI_RoutingServiceEnvironment_set_error(
                environment,
                "unexpected exception");
    }
}

RTI_RoutingServiceProcessor* PyProcessor::native()
{
    return &native_;
}


void PyProcessor::forward_on_route_event(
        void *native_processor_data,
        RTI_RoutingServiceRouteEvent *native_route_event,
        RTI_RoutingServiceEnvironment *environment)
{
//    if (PyErr_Occurred() != NULL) {
//        return;
//    }

    PyProcessor *forwarder =
            static_cast<PyProcessor*> (native_processor_data);

    try {
        // build up wrapper objects based on the event
        switch (RTI_RoutingServiceRouteEvent_get_kind(native_route_event)) {

        case RTI_ROUTING_SERVICE_ROUTE_EVENT_DATA_ON_INPUTS:
        {
            if (PyObject_CallMethod(
                    forwarder->py_processor_,
                    "on_data_available",
                    "O",
                    forwarder->py_route_) == NULL) {
                PyErr_Print();
                throw dds::core::Error("on_data_available: error calling Python processor");
            }

        }
            break;
//
//        case RTI_ROUTING_SERVICE_ROUTE_EVENT_PERIODIC_ACTION:
//
//            forwarder->processor_->on_periodic_action(forwarder->route());
//            break;

        case RTI_ROUTING_SERVICE_ROUTE_EVENT_INPUT_ENABLED:
        {
            void *affected_entity =
                    RTI_RoutingServiceRouteEvent_get_affected_entity(native_route_event);
            PyObjectGuard py_input = new PyInput(
                    static_cast<RTI_RoutingServiceStreamReaderExt *>(affected_entity),
                    forwarder->py_route_->get(),
                    environment);
            RTI_RoutingServiceRoute_set_stream_port_user_data(
                    forwarder->py_route_->get(),
                    static_cast<RTI_RoutingServiceStreamReaderExt *> (affected_entity)->stream_reader_data,
                    py_input.get());
            if (PyObject_CallMethod(
                    forwarder->py_processor_,
                    "on_input_enabled",
                    "OO",
                    forwarder->py_route_,
                    py_input.get()) == NULL) {
                PyErr_Print();
                throw dds::core::Error("on_input_enabled: error calling Python processor");
            }

            py_input.release();
        }
            break;


        case RTI_ROUTING_SERVICE_ROUTE_EVENT_INPUT_DISABLED:
        {

            void *affected_entity =
                    RTI_RoutingServiceRouteEvent_get_affected_entity(native_route_event);
            RTI_RoutingServiceStreamReaderExt *native_input =
                    static_cast<RTI_RoutingServiceStreamReaderExt *> (affected_entity);
            PyObjectGuard py_input =
                    forwarder->py_route_->input(native_input);
            if (PyObject_CallMethod(
                    forwarder->py_processor_,
                    "on_input_disabled",
                    "OO",
                    forwarder->py_route_,
                    py_input.get()) == NULL) {
                PyErr_Print();
                py_input.release();
                throw dds::core::Error("on_input_disabled: error calling Python processor");
            }

            RTI_RoutingServiceRoute_set_stream_port_user_data(
                    forwarder->py_route_->get(),
                    native_input->stream_reader_data,
                    NULL);
        }
            break;
        }

    } catch (const std::exception& ex) {
        RTI_RoutingServiceEnvironment_set_error(
                environment,
                "%s",
                ex.what());
    } catch (...) {
        RTI_RoutingServiceEnvironment_set_error(
                environment,
                "%s",
                "unexpected exception");
    }
}


/*
 * --- PyProcessorPlugin --------------------------------------------------
 */

void add_type(
    const char *name,
    PyTypeObject *type_object)
{
    if (PyType_Ready(type_object) < 0) {
        PyErr_Print();
        return;
    }
    Py_INCREF(type_object);
    PyObjectGuard type_guard = (PyObject *) type_object;

    // Import pyproc module
    PyObject *pyproc = PyImport_AddModule("pyproc");
    if (pyproc == NULL) {
        PyErr_Print();
        throw dds::core::Error(
                "load_module: error importing py proc module");
    }

    if (PyModule_AddObject(
            pyproc,
            name,
            (PyObject *) type_object) == -1) {
        PyErr_Print();
        return;
    }
}

PyObject* find_pyproc_type(const char *name)
{
    PyObject *pyproc_module = PyImport_AddModule("pyproc");
    if (pyproc_module == NULL) {
        PyErr_Print();
        throw dds::core::Error(
                "load_module: error importing pyproc module");
    }

    PyObject *pyproc_dict = PyModule_GetDict(pyproc_module);
    if (pyproc_dict == NULL) {
        PyErr_Print();
        throw dds::core::Error("load_module: error getting pyrpoc dictionary");
    }

    //PyObject_Print(plugin_class, stdout, 0);
    return PyDict_GetItemString(
            pyproc_dict,
            name);
}


PyProcessorPluginProperty::PyProcessorPluginProperty()
{

}


PyProcessorPluginProperty::PyProcessorPluginProperty(
        const std::string& class_name,
        const std::string& module_name = "")
        : class_name_(class_name),
        module_(module_name)
{

}

void PyProcessorPluginProperty::class_name(const std::string& class_name)
{
    class_name_ = class_name;
}

const std::string& PyProcessorPluginProperty::class_name()
{
    return class_name_;
}

void PyProcessorPluginProperty::module(const std::string& module_name)
{
    module_ = module_name;
}

const std::string& PyProcessorPluginProperty::module()
{
    return module_;
}

void PyProcessorPluginProperty::module_path(const std::string& module_path)
{
    module_path_ = module_path;
}

const std::string& PyProcessorPluginProperty::module_path()
{
    return module_path_;
}


const std::string PyProcessorPlugin::MODULE_PROPERTY_NAME =
        "rti.pyproc.module.name";

const std::string PyProcessorPlugin::MODULE_PATH_PROPERTY_NAME =
        "rti.pyproc.module.path";

const std::string PyProcessorPlugin::MODULE_PATH_VALUE_DEFAULT = ".";

const std::string PyProcessorPlugin::CLASS_NAME_PROPERTY_NAME =
        "rti.pyproc.class_name";

void PyProcessorPlugin::load_module()
{
    //   FILE *file = RTIOsapi_fileOpen("../PyProcessor.py", "r");
    //   if (file == NULL) {
    //       PyProcessor_log(PyExc_Exception, "error opening plugin file");
    //       throw dds::core::Error("load_module: error opening plugin file");
    //   }
    //   PyRun_SimpleFileEx(file, "", 1);
    //   if ( == NULL) {
    //       PyProcessor_log(PyExc_Exception, "error reading plugin file");
    //       throw dds::core::Error("load_module: error reading plugin file");
    //   }
    PyObject *main_module = PyImport_AddModule("__main__");
    if (main_module == NULL) {
        PyErr_Print();
        throw dds::core::Error("load_module: error addin main module");
    }
    PyObject *main_dict = PyModule_GetDict(main_module);
    if (main_dict == NULL) {
        PyErr_Print();
        throw dds::core::Error("load_module: error getting main dictionary");
    }
    //gobal_dict_ = PyDict_Copy(main_dict);

    // Import user module
    PyObject *user_module = PyImport_ImportModule(property_.module().c_str());
    if (user_module == NULL) {
        PyErr_Print();
        throw dds::core::Error(
                "load_module: error importing module="
                + property_.module());
    }
    PyObject *user_dict = PyModule_GetDict(user_module);
    if (user_dict == NULL) {
        PyErr_Print();
        throw dds::core::Error("load_module: error getting user dictionary");
    }

    // Add processor types to module
    add_type("Route", PyRouteType::type());
    add_type("Input", PyInputType::type());
    add_type("LoanedSamples", PyLoanedSamples::type());
    add_type("Sample", PySampleType::type());
    add_type("SampleData", PyDataType::type());
    add_type("SampleInfo", PyInfoType::type());


    //PyObject_Print(plugin_class, stdout, 0);
    create_processor_ = PyDict_GetItemString(
            user_dict,
            property_.class_name().c_str());
    if (create_processor_ == NULL) {
        PyErr_Print();
        throw dds::core::Error("load_module: create processor method not found");
    }
    if (!PyCallable_Check(create_processor_)) {
        throw dds::core::Error("load_module: create processor is not callable");
    }
}


PyProcessorPlugin::PyProcessorPlugin(
        const rti::routing::PropertySet& properties)
        : gobal_dict_(NULL),
          create_processor_(NULL)
{
    static PythonInitializer __python_init;

    // Check module properties
    property_.module_path(MODULE_PATH_VALUE_DEFAULT);


    PyList_Append(
            PySys_GetObject("path"),
            Py_BuildValue("s", "../"));

    for (auto it : properties) {
        if (it.first == MODULE_PATH_PROPERTY_NAME) {
            property_.module_path(it.second);
        } else if (it.first == MODULE_PROPERTY_NAME) {
            property_.module(it.second);
        } else if (it.first == CLASS_NAME_PROPERTY_NAME) {
            property_.class_name(it.second);
        }
    }

    PyObject *sys_path = PySys_GetObject("path");
    assert(sys_path != NULL);
    PyObject *py_module_path = Py_BuildValue("s", property_.module_path().c_str());
    int append_result = PyList_Append(
            sys_path,
            py_module_path);
    Py_DECREF(py_module_path);
    if (append_result == -1) {
        PyErr_Print();
        throw dds::core::Error("PyProcessorPlugin: add module path");
    }

    load_module();
}

PyProcessorPlugin::~PyProcessorPlugin()
{
    //Py_DECREF(gobal_dict_);
}

PyObject* PyProcessorPlugin::create_processor(
        PyRoute *py_route,
        PyObject *py_properties)
{
    PyObject *py_proc = PyObject_CallMethod(
            create_processor_,
            "create_processor",
            "OO",
            py_route,
            py_properties);
    if (py_proc == NULL) {
        PyErr_Print();
        throw dds::core::Error("PyProcessorPlugin: failed to create processor from plugin");
    }
    if (!PyObject_IsInstance(py_proc, find_pyproc_type("Processor"))) {
        PyErr_Print();
        Py_DECREF(py_proc);
        throw dds::core::Error("PyProcessorPlugin: create_processor must return an implementation of Processor");
    }

    return py_proc;
}

RTI_RoutingServiceProcessorPlugin *
PyProcessorPlugin::create_plugin(
        PyProcessorPlugin* plugin)
{
    RTI_RoutingServiceProcessorPlugin *native_plugin = NULL;
    RTIOsapiHeap_allocateStructure(
            &native_plugin,
            struct RTI_RoutingServiceProcessorPlugin);
    rti::core::check_create_entity(
            native_plugin,
            "RTI_RoutingServiceProcessorPlugin");
    RTI_RoutingServiceProcessorPlugin_initialize(native_plugin);

    // TODO: retrieve version from python

    // Initialize native implementation
    native_plugin->processor_plugin_data =
            static_cast<void *> (plugin);
    native_plugin->plugin_delete =
            PyProcessorPlugin::delete_plugin;
    native_plugin->create_processor =
            PyProcessorPlugin::forward_create_processor;
    native_plugin->delete_processor =
            PyProcessorPlugin::forward_delete_processor;

    return native_plugin;
}

void PyProcessorPlugin::delete_plugin(
        RTI_RoutingServiceProcessorPlugin* native_plugin,
        RTI_RoutingServiceEnvironment*)
{
    PyProcessorPlugin *plugin = static_cast<PyProcessorPlugin*> (
            native_plugin->processor_plugin_data);
    // Plug-in is destructor not allowed to throw
    delete plugin;
    RTIOsapiHeap_freeStructure(native_plugin);
}

RTI_RoutingServiceProcessor *
PyProcessorPlugin::forward_create_processor(
        void *native_plugin_data,
        RTI_RoutingServiceRoute *native_route,
        const struct RTI_RoutingServiceProperties *native_properties,
        RTI_RoutingServiceEnvironment *environment)
{
    return PyProcessor::create_native(
            static_cast<PyProcessorPlugin *> (native_plugin_data),
            native_route,
            native_properties,
            environment);
}

void PyProcessorPlugin::forward_delete_processor(
        void *native_plugin_data,
        struct RTI_RoutingServiceProcessor *native_processor,
        RTI_RoutingServiceRoute *,
        RTI_RoutingServiceEnvironment *environment)
{

    PyProcessor::delete_native(
            static_cast<PyProcessorPlugin *> (native_plugin_data),
            native_processor,
            environment);
}



//
//    static void delete_native(
//            ProcessorPlugin *plugin,
//            RTI_RoutingServiceProcessor *native_processor,
//            RTI_RoutingServiceEnvironment *environment)
//    {
//        ProcessorForwarder *processor_forwarder =
//                static_cast<ProcessorForwarder*>(native_processor->processor_data);
//        try {
//            if (processor_forwarder->processor_ != NULL) {
//                plugin->delete_processor(
//                        processor_forwarder->route_,
//                        processor_forwarder->processor_);
//                processor_forwarder->processor_ = NULL;
//            }
//        } catch(const std::exception& ex) {
//            RTI_RoutingServiceEnvironment_set_error(
//                    environment,
//                    "%s",
//                    ex.what());
//        } catch (...) {
//            RTI_RoutingServiceEnvironment_set_error(
//                    environment,
//                    "unexpected exception");
//        }
//
//        delete processor_forwarder;
//    }
//
//
//    static void forward_on_route_event(
//        void *native_processor_data,
//        RTI_RoutingServiceRouteEvent *native_route_event,
//        RTI_RoutingServiceEnvironment *environment)
//    {
//
//        ProcessorForwarder *forwarder =
//                static_cast<ProcessorForwarder*>(native_processor_data);
//
//        try {
//
//            // build up wrapper objects based on the event
//            switch (RTI_RoutingServiceRouteEvent_get_kind(native_route_event)) {
//
//            case RTI_ROUTING_SERVICE_ROUTE_EVENT_DATA_ON_INPUTS:
//                forwarder->processor_->on_data_available(forwarder->route());
//                break;
//
//            case RTI_ROUTING_SERVICE_ROUTE_EVENT_PERIODIC_ACTION:
//
//                forwarder->processor_->on_periodic_action(forwarder->route());
//                break;
//
//            case RTI_ROUTING_SERVICE_ROUTE_EVENT_INPUT_ENABLED:
//            {
//                void *affected_entity =
//                        RTI_RoutingServiceRouteEvent_get_affected_entity(native_route_event);
//                void *index =
//                        RTI_RoutingServiceRouteEvent_get_event_data(native_route_event);
//                ScopedPort<Input, RTI_RoutingServiceStreamReaderExt> port(
//                        forwarder->route_,
//                        *(static_cast<int32_t*>(index)),
//                        static_cast<RTI_RoutingServiceStreamReaderExt *>(affected_entity));
//                forwarder->processor_->on_input_enabled(
//                        forwarder->route(),
//                        *port.get());
//                RTI_RoutingServiceRoute_set_stream_port_user_data(
//                        forwarder->route_.native_route_,
//                        static_cast<RTI_RoutingServiceStreamReaderExt *>(affected_entity)->stream_reader_data,
//                        port.get());
//                port.clear();
//
//            }
//                break;
//
//            case RTI_ROUTING_SERVICE_ROUTE_EVENT_INPUT_DISABLED:
//            {
//
//                void *affected_entity =
//                        RTI_RoutingServiceRouteEvent_get_affected_entity(native_route_event);
//                RTI_RoutingServiceStreamReaderExt *native_input =
//                        static_cast<RTI_RoutingServiceStreamReaderExt *>(affected_entity);
//                Input *input = reinterpret_cast<Input*>(
//                        RTI_RoutingServiceRoute_get_stream_port_user_data(
//                                forwarder->route_.native_route_,
//                                native_input->stream_reader_data));
//                forwarder->processor_->on_input_disabled(
//                        forwarder->route(),
//                        *input);
//                RTI_RoutingServiceRoute_set_stream_port_user_data(
//                        forwarder->route_.native_route_,
//                        native_input->stream_reader_data,
//                        NULL);
//                delete input;
//            }
//                break;
//
//            case RTI_ROUTING_SERVICE_ROUTE_EVENT_OUTPUT_ENABLED:
//            {
//                void *affected_entity =
//                        RTI_RoutingServiceRouteEvent_get_affected_entity(native_route_event);
//                void *index =
//                        RTI_RoutingServiceRouteEvent_get_event_data(native_route_event);
//                ScopedPort<Output, RTI_RoutingServiceStreamWriterExt> port(
//                        forwarder->route_,
//                        *(static_cast<int32_t*>(index)),
//                        static_cast<RTI_RoutingServiceStreamWriterExt *>(affected_entity));
//                forwarder->processor_->on_output_enabled(
//                        forwarder->route(),
//                        *port.get());
//                RTI_RoutingServiceRoute_set_stream_port_user_data(
//                        forwarder->route_.native_route_,
//                        static_cast<RTI_RoutingServiceStreamWriterExt *>(affected_entity)->stream_writer_data,
//                        port.get());
//                port.clear();
//
//            }
//                break;
//
//            case RTI_ROUTING_SERVICE_ROUTE_EVENT_OUTPUT_DISABLED:
//            {
//                void *affected_entity =
//                        RTI_RoutingServiceRouteEvent_get_affected_entity(native_route_event);
//                RTI_RoutingServiceStreamWriterExt *native_output =
//                        static_cast<RTI_RoutingServiceStreamWriterExt *>(affected_entity);
//                Output *output = reinterpret_cast<Output*>(
//                        RTI_RoutingServiceRoute_get_stream_port_user_data(
//                                forwarder->route_.native_route_,
//                                native_output->stream_writer_data));
//                forwarder->processor_->on_output_disabled(
//                        forwarder->route(),
//                        *output);
//                RTI_RoutingServiceRoute_set_stream_port_user_data(
//                        forwarder->route_.native_route_,
//                        native_output->stream_writer_data,
//                        NULL);
//                delete output;
//            }
//                break;
//
//            case RTI_ROUTING_SERVICE_ROUTE_EVENT_ROUTE_STARTED:
//
//                forwarder->processor_->on_start(forwarder->route());
//                break;
//
//            case RTI_ROUTING_SERVICE_ROUTE_EVENT_ROUTE_STOPPED:
//
//                forwarder->processor_->on_stop(forwarder->route());
//                break;
//
//            case RTI_ROUTING_SERVICE_ROUTE_EVENT_ROUTE_RUNNING:
//
//                forwarder->processor_->on_run(forwarder->route());
//                break;
//
//            case RTI_ROUTING_SERVICE_ROUTE_EVENT_ROUTE_PAUSED:
//
//                forwarder->processor_->on_pause(forwarder->route());
//                break;
//
//            default:
//                break;
//            };
//
//        } catch (const std::exception& ex) {
//            RTI_RoutingServiceEnvironment_set_error(
//                    environment,
//                    "%s",
//                    ex.what());
//        } catch (...) {
//            RTI_RoutingServiceEnvironment_set_error(
//                    environment,
//                    "%s",
//                    "unexpected exception");
//        }
//    }
//
//    static void forward_update(
//            void *native_processor_data,
//            const struct RTI_RoutingServiceProperties *native_properties,
//            RTI_RoutingServiceEnvironment *environment)
//    {
//
//        ProcessorForwarder *processorForwarder =
//                static_cast<ProcessorForwarder*> (native_processor_data);
//
//        rti::routing::detail::UpdatableEntityForwarder::update(
//                processorForwarder->processor_,
//                native_properties,
//                environment);
//    }
//
//    RTI_RoutingServiceProcessor* native()
//    {
//        return &native_;
//    }
//
//
//
//

struct RTI_RoutingServiceProcessorPlugin * PyProcessorPlugin_create_processor_plugin(
        const struct RTI_RoutingServiceProperties * native_properties,
        RTI_RoutingServiceEnvironment *environment)
{
    PropertySet properties;
    rti::routing::PropertyAdapter::add_properties_from_native(
            properties,
            native_properties);
    try {
        return PyProcessorPlugin::create_plugin(new PyProcessorPlugin(properties));
    } catch (const std::exception& ex) {
        RTI_RoutingServiceEnvironment_set_error(
                environment,
                "%s",
                ex.what());
    } catch (...) {}

    return NULL;
}

} } }
