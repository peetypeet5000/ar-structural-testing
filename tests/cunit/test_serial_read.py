from ctypes import CDLL

import pytest
from tests.cunit import SRC, compile

C = CDLL("libc.so.6")

@pytest.fixture
def serial_read():
    source = SRC / "serial_read.c"
    compile(source)
    yield CDLL(str(source.with_suffix(".so")))

def test_read_serial_data(serial_read):
    # TO DO - add unit tests
    # possible resource? 
    # https://pythonhosted.org/pyserial/tools.html#miniterm
