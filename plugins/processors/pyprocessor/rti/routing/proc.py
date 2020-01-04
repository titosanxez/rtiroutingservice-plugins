from abc import ABC, abstractmethod
from enum import IntEnum

class Processor(ABC):
    @abstractmethod
    def __init__(self, route, properties):
        NotImplemented

    def on_input_enabled(self, route, input):
        NotImplemented

    def on_input_disabled(self, route, input):
        NotImplemented

    def on_output_enabled(self, route, output):
        NotImplemented

    def on_output_disabled(self, route, output):
        NotImplemented

    def on_start(self, route):
        NotImplemented

    def on_stop(self, route):
        NotImplemented

    def on_run(self, route):
        NotImplemented

    def on_pause(self, route):
        NotImplemented

    def on_data_available(self, route):
        NotImplemented

    def on_periodic_event(self, route):
        NotImplemented
