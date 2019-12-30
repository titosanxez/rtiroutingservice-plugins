import os
import sys
import time

sys.path.insert(0, os.path.abspath('..'))

from rti.routing import proc

class ShapesProcessor(proc.Processor):
    def __init__(self, route, properties):
        pass

    def on_data_available(self, route):
        # Use squares as 'leading' input. For each Square instance, get the
        # equivalent instance from the Circle topic
        squares = route["Square"].read()
        for square in squares:
            if square.info['valid_data']:
                # read equivalent existing instance in the Circles Topic
                selector = dict(instance=square.info['instance_handle'])
                circles = route['Circle'].read(selector)
                if len(circles) != 0 and circles[0].info['valid_data'] :
                    square.data['shapesize'] += circles[0].data['y']

                route['Triangle'].write(square.data)
            else:
                # dispose instance
                route['Triangle'].write(square.data, square.info)
                # clear cache
                route['Square'].take(dict(instance=square.info['instance_handle']))



class ShapesProcessorPlugin(proc.ProcessorPlugin):
    def create_processor(route, properties):
        return ShapesProcessor(route, properties)

