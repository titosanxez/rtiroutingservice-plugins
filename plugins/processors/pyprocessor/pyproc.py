from abc import ABC, abstractmethod
from enum import IntEnum

class Processor(ABC):
    @abstractmethod
    def __init__(self, route, properties):
        NotImplemented

    @abstractmethod
    def on_input_enabled(self, route, input):
        NotImplemented

    @abstractmethod
    def on_input_disabled(self, route, input):
        NotImplemented

    @abstractmethod
    def on_data_available(self, route):
        NotImplemented


class ProcessorPlugin(ABC):

    @classmethod
    @abstractmethod
    def create_processor(route, properties):
        return NotImplemented

#def create_processor():
#    return 1

#
#if __name__ == '__main__':
#    print('Hello World')
