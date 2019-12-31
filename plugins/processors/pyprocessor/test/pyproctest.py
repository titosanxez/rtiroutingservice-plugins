import os
import sys
import time
import json
import pprint

sys.path.insert(0, os.path.abspath('..'))

from rti.routing import proc

class MyProcessor(proc.Processor):
    def __init__(self, route, properties):
        pass

    def on_data_available(self, route):
        try:
            print('on_data_available')
            samples = route.inputs['Square'].read();
            for sample in samples:
                pprint.PrettyPrinter(indent=4).pprint(sample.data)
                #print(json.dumps(sample.data))
                route.outputs['Triangle'].write(sample.data)
        except AttributeError as atterr:
            print(atterr)
        except TypeError as typerr:
            print(typerr)



class MyProcessorPlugin(proc.ProcessorPlugin):
    def create_processor(route, properties):
        #print(route.read_at(0))
        return MyProcessor(route, properties)


#def create_processor():
#    print('Hello World')
