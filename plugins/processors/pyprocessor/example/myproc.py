import os
import sys
import time

sys.path.insert(0, os.path.abspath('..'))

from rti.routing import proc

class MyProcessor(proc.Processor):
    def __init__(self, route, properties):
        self.my_data = 'MyProcessor'
        print('MyProcessor constructor called')

    def on_input_enabled(self, route, input):
        print('input_at: ' + route.input(0).name)
        print('on_input_enabled: ' + self.my_data + ' input=' + input.name \
            + ', stream=' + input.stream_name)

    def on_input_disabled(self, route, input):
        print('on_input_disabled: ' + self.my_data + ' input=' + input.name)

    def on_output_enabled(self, route, output):
        print('output_at: ' + route.output(0).name)
        print('on_output_enabled: ' + self.my_data + ' output=' + output.name \
             + ', stream=' + output.stream_name)

    def on_output_disabled(self, route, output):
       print('on_output_disabled: ' + self.my_data + ' output=' + output.name)

    def on_data_available(self, route):
        try:
            print('on_data_available: ' + self.my_data)
            samples = route.input(0).read();
            print(len(samples))
            print(samples[0].data)
            #print(samples[0].info['instance_handle'])
            route.output(0).write(samples[0].data);
            time.sleep(1)
            samples = route.input(0).read(\
                dict(instance=samples[0].info['instance_handle']));
            print(samples[0].data)
#            samples[0].data = 0;
        except AttributeError as atterr:
            print(atterr)
        except TypeError as typerr:
            print(typerr)



class MyProcessorPlugin(proc.ProcessorPlugin):
    def create_processor(route, properties):
        print('MyProcessorPlugin.create_processor')
        print(properties)
        print(route.name())
        #print(route.read_at(0))
        return MyProcessor(route, properties)

#def create_processor():
#    print('Hello World')
