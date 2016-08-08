# -*- coding: utf-8 -*-

"""
sqlite-uuid test suite
"""

import collections
import os.path
import uuid

import pytest

try:
    from pysqlite2 import dbapi2 as sqlite3
except ImportError:
    import sqlite3

NamespaceExample = collections.namedtuple('NamespaceExample', ['namespace', 'name'])


@pytest.fixture
def db(extension):
    conn = sqlite3.connect(':memory:')
    conn.enable_load_extension(True)
    conn.load_extension(extension)
    conn.enable_load_extension(False)
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


def query(db, *args):
    return db.execute(*args).fetchone()[0]


def test_uuid1(db):
    result = query(db, 'SELECT uuid1();')
    u = uuid.UUID(result)
    assert u.version == 1
    assert u.variant == 'specified in RFC 4122'


def test_uuid3(db, uuid3_examples):
    for example in uuid3_examples:
        result = query(db, 'SELECT uuid3(?, ?);', example[0])
        u = uuid.UUID(result)
        assert u == example[1]
        assert u.version == 3
        assert u.variant == 'specified in RFC 4122'


def test_uuid4(db):
    result = db.execute('SELECT uuid4();').fetchone()[0]
    u = uuid.UUID(result)
    assert u.version == 4
    assert u.variant == 'specified in RFC 4122'


def test_uuid5(db, uuid5_examples):
    for example in uuid5_examples:
        result = query(db, 'SELECT uuid5(?, ?);', example[0])
        u = uuid.UUID(result)
        assert u == example[1]
        assert u.version == 5
        assert u.variant == 'specified in RFC 4122'


def test_uuid_ns_dns(db):
    result = query(db, 'SELECT uuid_ns_dns();')
    u = uuid.UUID(result)
    assert u == uuid.NAMESPACE_DNS


def test_uuid_ns_oid(db):
    result = query(db, 'SELECT uuid_ns_oid();')
    u = uuid.UUID(result)
    assert u == uuid.NAMESPACE_OID


def test_uuid_ns_url(db):
    result = query(db, 'SELECT uuid_ns_url();')
    u = uuid.UUID(result)
    assert u == uuid.NAMESPACE_URL


def test_uuid_ns_x500(db):
    result = query(db, 'SELECT uuid_ns_x500();')
    u = uuid.UUID(result)
    assert u == uuid.NAMESPACE_X500
