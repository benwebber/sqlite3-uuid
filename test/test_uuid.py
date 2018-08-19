# -*- coding: utf-8 -*-

"""
sqlite-uuid test suite
"""

import secrets
import uuid

import apsw
import pytest


def query(db, *args):
    with db:
        return db.cursor().execute(*args).fetchone()[0]


class TestUUIDv3:
    def test_uuid3(self, db, uuid3_examples):
        """
        Should return expected v3 UUID for known input.
        """
        for example in uuid3_examples:
            args, expected = example
            result = query(db, 'SELECT uuid3(?, ?);', args)
            u = uuid.UUID(result)
            assert u == expected
            assert u.version == 3
            assert u.variant == uuid.RFC_4122

    def test_uuid3_invalid_namespace(self, db):
        """
        Should raise an error for invalid or namespace UUID.
        """
        with pytest.raises(apsw.SQLError) as exc:
            query(db, 'SELECT uuid3(?, ?);', ('this is not a UUID', 'test'))
        assert 'cannot parse namespace UUID' in str(exc)


class TestUUIDv5:
    def test_uuid5(self, db, uuid5_examples):
        """
        Should return expected v5 UUID for known input.
        """
        for example in uuid5_examples:
            args, expected = example
            result = query(db, 'SELECT uuid5(?, ?);', args)
            u = uuid.UUID(result)
            assert u == expected
            assert u.version == 5
            assert u.variant == uuid.RFC_4122

    def test_uuid5_invalid_namespace(self, db):
        """
        Should raise an error for invalid or namespace UUID.
        """
        with pytest.raises(apsw.SQLError) as exc:
            query(db, 'SELECT uuid5(?, ?);', ('this is not a UUID', 'test'))
        assert 'cannot parse namespace UUID' in str(exc)

class TestUUIDConversion:
    def test_uuid_to_text(self, db, uuid5_examples):
        """
        Should convert binary UUID (bytes) to canonical string representation.
        """
        for _, expected in uuid5_examples:
            result = query(db, 'SELECT uuid_to_text(?);', (expected.bytes,))
            u = uuid.UUID(result)
            assert u == expected

    def test_uuid_to_text_invalid_length(self, db, uuid5_examples):
        """
        Should raise an error if binary UUID data is not 16 bytes long.
        """
        with pytest.raises(apsw.SQLError) as exc:
            buf = secrets.token_bytes(15)
            query(db, 'SELECT uuid_to_text(?);', (buf,))
        assert 'UUID must be 16 bytes' in str(exc)

    def test_uuid_to_blob(self, db, uuid5_examples):
        """
        Should convert canonical string UUID to binary (bytes).
        """
        for _, expected in uuid5_examples:
            result = query(db, 'SELECT uuid_to_blob(?);', (str(expected),))
            u = uuid.UUID(bytes=result)
            assert u == expected

    def test_uuid_to_blob_invalid_uuid(self, db):
        """
        Should raise an error if string is not UUID.
        """
        with pytest.raises(apsw.SQLError) as exc:
            query(db, 'SELECT uuid_to_blob(?);', ('foo',))
        assert 'cannot parse UUID' in str(exc)


def test_uuid1(db):
    result = query(db, 'SELECT uuid1();')
    u = uuid.UUID(result)
    assert u.version == 1
    assert u.variant == uuid.RFC_4122


def test_uuid4(db):
    result = query(db, 'SELECT uuid4();')
    u = uuid.UUID(result)
    assert u.version == 4
    assert u.variant == uuid.RFC_4122


def test_uuid_nil(db):
    result = query(db, 'SELECT uuid_nil();')
    u = uuid.UUID(result)
    assert u == uuid.UUID('00000000-0000-0000-0000-000000000000')
    assert u.variant == uuid.RESERVED_NCS


@pytest.mark.parametrize('sql, expected', [
    ('SELECT uuid_ns_dns();', uuid.NAMESPACE_DNS),
    ('SELECT uuid_ns_oid();', uuid.NAMESPACE_OID),
    ('SELECT uuid_ns_url();', uuid.NAMESPACE_URL),
    ('SELECT uuid_ns_x500();', uuid.NAMESPACE_X500),
])
def test_namespace_uuids(db, sql, expected):
    result = query(db, sql)
    assert uuid.UUID(result) == expected
