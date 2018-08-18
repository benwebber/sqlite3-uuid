# -*- coding: utf-8 -*-

"""
sqlite-uuid test suite
"""

import uuid


def query(db, *args):
    with db:
        return db.cursor().execute(*args).fetchone()[0]


def test_uuid1(db):
    result = query(db, 'SELECT uuid1();')
    u = uuid.UUID(result)
    assert u.version == 1
    assert u.variant == uuid.RFC_4122


def test_uuid3(db, uuid3_examples):
    for example in uuid3_examples:
        args, expected = example
        result = query(db, 'SELECT uuid3(?, ?);', args)
        u = uuid.UUID(result)
        assert u == expected
        assert u.version == 3
        assert u.variant == uuid.RFC_4122


def test_uuid4(db):
    result = query(db, 'SELECT uuid4();')
    u = uuid.UUID(result)
    assert u.version == 4
    assert u.variant == uuid.RFC_4122


def test_uuid5(db, uuid5_examples):
    for example in uuid5_examples:
        args, expected = example
        result = query(db, 'SELECT uuid5(?, ?);', args)
        u = uuid.UUID(result)
        assert u == expected
        assert u.version == 5
        assert u.variant == uuid.RFC_4122


def test_uuid_nil(db):
    result = query(db, 'SELECT uuid_nil();')
    u = uuid.UUID(result)
    assert u == uuid.UUID('00000000-0000-0000-0000-000000000000')
    assert u.variant == uuid.RESERVED_NCS


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


def test_uuid_to_text(db, uuid5_examples):
    for _, expected in uuid5_examples:
        result = query(db, 'SELECT uuid_to_text(?);', (expected.bytes,))
        u = uuid.UUID(result)
        assert u == expected


def test_uuid_to_blob(db, uuid5_examples):
    for _, expected in uuid5_examples:
        result = query(db, 'SELECT uuid_to_blob(?);', (str(expected),))
        u = uuid.UUID(bytes=result)
        assert u == expected
