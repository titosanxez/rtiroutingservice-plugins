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
        print('input_at: ' + route.inputs(0).info['name'])
        print('on_input_enabled: ' + self.my_data + ' input=' + str(input.info))

    def on_input_disabled(self, route, input):
        print('on_input_disabled: ' + self.my_data + ' input=' + input.info['name'])

    def on_output_enabled(self, route, output):
        print('output_at: ' + route.outputs(0).info['name'])
        print('on_output_enabled: ' + self.my_data + ' output=' + str(output.info))

    def on_output_disabled(self, route, output):
       print('on_output_disabled: ' + self.my_data + ' output=' + output.info['name'])

    def on_data_available(self, route):
        try:
            print('on_data_available: ' + self.my_data)
            samples = route.inputs('Square').read();
            print(len(samples))
            print(samples[0].data[0])
            #print(samples[0].info['instance_handle'])
            route.outputs(0).write(samples[0].data);
            time.sleep(1)
            selector = dict(\
                instance=samples[0].info['instance_handle'],
                max_samples=5)
            samples = route.inputs(0).read(selector)
            print(samples[0].data)
            print(len(samples))
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
