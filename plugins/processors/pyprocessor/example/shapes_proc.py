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
        squares = route.inputs["Square"].read()
        for shape in squares:
            if shape.valid_data:
                # read equivalent existing instance in the Circles Topic

                selector = dict(instance=shape.info['instance_handle'])
                circles = route.outputs['Circle'].read(selector)
                if len(circles) != 0 and circles[0].valid_data :
                    shape.data['shapesize'] = circles[0].data['y']

                route['Triangle'].write(shape.data)
            else:
                # propagate dispose/unregister instance
                route['Triangle'].write(shape)
                # clear cache
                route['Square'].take(dict(instance=shape.info['instance_handle']))


#3) In Connector we have for square in squares.valid_data_iter: so you don’t have to check if square.valid_data.