import os.path

import pytest

def pytest_addoption(parser):
    parser.addoption(
        '--extension',
        action='store',
        help='path to sqlite3-uuid shared library',
        default=os.path.join(os.path.dirname(__file__), '../dist/uuid'),
    )


@pytest.fixture
def extension(request):
    return request.config.getoption('--extension')
