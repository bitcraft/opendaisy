"""
unused python prototyping
"""

class Stepper:
    """ Stepper motor abstractions

    Python timing and general OS limitations means that running a
    stepper from GPIO is a bad time.  Move to a microcontroller.

    Modeled after AccelStepper for arduino
    http://www.airspayce.com/mikem/arduino/AccelStepper/

    Open Source Licensing GPL V2
    """
    DRIVER = 1      # use stepper driver
    FULL2WIRE = 2  # 2 wire stepper
    FULL3WIRE = 3  # 3 wire stepper, such as a HDD spindle
    FULL4WIRE = 4  # 4 wire stepper
    HALF3WIRE = 6  # half stepped 3 wire
    HALF4WIRE = 8  # half stepped 4 wire

    def __init__(self):
        self.backlash = 0
        self.steps_per_revolution = 200
        self.polarity = None  # bipolar or unipolar
        self.max_speed = 200  # steps/second
        self.acceleration = 0  # steps/second
        self.pulse = 20  # microseconds
        self._position = 0
        self._target = 0
        self._speed = 0

    async def move(self, value):
        """ Move x steps forwards or backwards

        :param value:
        :return:
        """
        await self.move_to(self._position + value)

    async def move_to(self, value):
        """ Move to position x.  This is relative to home position.

        Home position is zero when initialized.  Set home to known
        physical position for repeatable positioning.

        :param value:
        :return:
        """
        self._target = value
        self.compute_new_speed()
        await self.run_blocking()

    async def run_blocking(self):
        """ Update the stepper and wait until it has reached target position

        :return:
        """
        pass

    @property
    def position(self):
        return self._position

    @property
    def target(self):
        return self._target

    @property
    def distance(self):
        return self._target - self._position

    @property
    def running(self):
        # return not (speed and self._target == self._position)
        pass

    def compute_new_speed(self):
        # speed =
        # steps = (speed * speed) / (2.0 * accel)
        pass

    def set_home(self):
        """ Mark current position as home

        :return:
        """
        self._position = 0

    def stop(self):
        """ Stop the stepper, no matter what it is doing

        :return:
        """
        pass

    def step_forward(self):
        """ Step once forward

        :return:
        """
        # TODO: stepper types
        self._position += 1

    def step_reverse(self):
        """ Step once in reverse

        :return:
        """
        # TODO: stepper types
        self._position -= 1
