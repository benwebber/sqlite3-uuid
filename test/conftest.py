import collections
import os.path
import uuid
from typing import NamedTuple

import apsw
import pytest


class NamespaceExample(NamedTuple):
    namespace: uuid.UUID
    name: str


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


@pytest.fixture
def db(extension):
    conn = apsw.Connection(':memory:')
    conn.enableloadextension(True)
    conn.loadextension(extension)
    conn.enableloadextension(False)
    return conn


@pytest.fixture
def examples():
    return [
        NamespaceExample(uuid.NAMESPACE_DNS, 'example.org'),
        NamespaceExample(uuid.NAMESPACE_DNS, 'www.example.org'),
        NamespaceExample(uuid.NAMESPACE_OID, '0.1.2.3'),
        NamespaceExample(uuid.NAMESPACE_OID, '0.1.2.3.4'),
        NamespaceExample(uuid.NAMESPACE_URL, 'https://example.org'),
        NamespaceExample(uuid.NAMESPACE_URL, 'https://www.example.org'),
        NamespaceExample(uuid.NAMESPACE_X500, 'cn=www.example.org,ou=Technology,o=Internet Corporation for Assigned Names and Numbers,L=Los Angeles,ST=California,C=US'),
    ]


@pytest.fixture
def uuid3_examples(examples):
    return [((str(e.namespace), e.name), uuid.uuid3(e.namespace, e.name))
            for e in examples]


@pytest.fixture
def uuid5_examples(examples):
    return [((str(e.namespace), e.name), uuid.uuid5(e.namespace, e.name))
            for e in examples]
