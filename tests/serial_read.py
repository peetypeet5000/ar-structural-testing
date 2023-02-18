from ctypes import CDLL

import pytest

@pytest.fixture
def lib_read_serial_data():
    yield CDLL("./serial_read.so")

def test_read_serial_data(lib_read_serial_data):
    # TO DO - ADD UNIT TESTS
