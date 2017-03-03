class DiasyWheel:
    """

    """

    def __init__(self):
        self.homed = False

    def find_home(self):
        pass

    def set_glyph(self):
        pass


class Typeface:
    def __init__(self):
        self.glyphs = []
        self.orientation = 0  # clockwise or counter-clockwise
        self.rings = 1        # can be shifter


class Glyph:
    """ Represents a single element that can be impressed onto paper,
        typically a letter, number, or other human legible symbol.

    """

    def __init__(self):
        self.value = None


class Solenoid:
    def __init__(self):
        self.on_timing = 0  # time from off state to on state
        self.off_timing = 0  # time from on state to off state
        self.state = 0

    async def engage(self):
        # yield loop sleep time
        self.state = 1

    async def disengage(self):
        # yield loop sleep time
        self.state = 0


class Carriage:
    def __init__(self):
        self.position = 0
        self.homed = False


class MotorDriver:
    def __init__(self):
        pass


class Document:
    def __init__(self):
        pass


class Buffer:
    def __init__(self):
        pass


class Color:
    def __init__(self):
        self.name = None
        self.rgba = 0, 0, 0, 0


class RibbonStripe:
    def __init__(self):
        self.color = None


class Ribbon:
    def __init__(self):
        self.stripes = []  # may have more for different colors
        self.height = 1  # real world measurement
        self.length = 0


class RibbonHolder:
    def __init__(self):
        pass


class Platten:
    def __init__(self):
        self.length = 13
        self.radius = 1


class Switch:
    """ Switch has state on or off

    """

    def __init__(self):
        self.state = False  # true or false


class Keyboard:
    """

    """

    def __init__(self):
        self.keys = []
        self.layout = None


class KeyboardKey:
    def __init__(self):
        self.value = None  # basically, the symbol etched onto the key


class SwitchMatrix:
    """ Convert digital signals to values

    Form of encoding using rows and columns

    """

    def __init__(self):
        pass

    def scan(self):
        pass

    def decode(self, rows, columns):
        pass


class Beeper:
    pass


class Typewriter:
    """ High level interface to typewriter

    implements some ascii events

    """

    def __init__(self):
        pass

    def type_letter(self, value):
        pass

    def type_string(self, value):
        pass

    def newline(self):
        pass

    def bell(self):
        """ 0x07 BEL

        :return:
        """
        pass

    def backspace(self):
        """ 0x08 BS

        :return:
        """
        pass

    def tab(self):
        """ 0x09 HT

        :return:
        """
        pass

    # factor out below into generic classes
    def run_once(self):
        # scan inputs
        # fill buffer with inputs
        # decode events
        # process decoded events
        # clear processed events
        pass


class Interface:
    """ Glue between API and hardware, kinda

    """

    def __init__(self):
        pass



