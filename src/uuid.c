/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This SQLite extension implements functions for creating RFC 4122 compliant
** UUIDs.
**
*/
#include <sqlite3ext.h>
SQLITE_EXTENSION_INIT1
#include <assert.h>
#include <uuid/uuid.h>
#include <string.h>

#ifdef USE_OPENSSL
#include <openssl/md5.h>
#include <openssl/sha.h>
#endif

#ifdef USE_COMMONCRYPTO
#define COMMON_DIGEST_FOR_OPENSSL
#include <CommonCrypto/CommonDigest.h>
#endif

#define SET_VARIANT(uu)           (uu[8] = (uu[8] & 0xbf) | 0x80)
#define SET_VERSION(uu, version)  (uu[6] = (uu[6] & 0x0f) | (version << 4))

#define UUID_LENGTH  36
#define UUID_SIZE    16

/*
** Implementation of uuid1() function.
*/
static void uuid1func(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  assert(argc==0);
  uuid_t uuid;
  uuid_generate_time(uuid);
  char uuid_str[37];
  uuid_unparse_lower(uuid, uuid_str);
  sqlite3_result_text(context, uuid_str, UUID_LENGTH, SQLITE_TRANSIENT);
}

/*
** Implementation of the uuid3() function.
*/
static void uuid3func(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  assert(argc==2);
  uuid_t namespace_uuid;
  char uuid_str[37];

  const unsigned char *namespace = sqlite3_value_text(argv[0]);
  const unsigned char *name      = sqlite3_value_text(argv[1]);

  if(uuid_parse((const char *)namespace, namespace_uuid) == -1) {
    sqlite3_result_error(context, "invalid uuid", -1);
    return;
  }

  uuid_t uu;

  MD5_CTX mdctx;
  unsigned char md_value[MD5_DIGEST_LENGTH];
  MD5_Init(&mdctx);
  MD5_Update(&mdctx, namespace_uuid, UUID_SIZE);
  MD5_Update(&mdctx, name, strlen((const char *)name));
  MD5_Final(md_value, &mdctx);

  SET_VARIANT(md_value);
  SET_VERSION(md_value, 3);

  memcpy(uu, md_value, UUID_SIZE);

  uuid_unparse_lower(uu, uuid_str);
  sqlite3_result_text(context, uuid_str, UUID_LENGTH, SQLITE_TRANSIENT);
}

/*
** Implementation of the uuid4() function.
*/
static void uuid4func(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  assert(argc==0);
  uuid_t uuid;
  uuid_generate_random(uuid);
  char uuid_str[37];
  uuid_unparse_lower(uuid, uuid_str);
  sqlite3_result_text(context, uuid_str, UUID_LENGTH, SQLITE_TRANSIENT);
}

/*
** Implementation of the uuid5() function.
*/
static void uuid5func(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  assert(argc==2);
  uuid_t namespace_uuid;
  char uuid_str[37];

  const unsigned char *namespace = sqlite3_value_text(argv[0]);
  const unsigned char *name      = sqlite3_value_text(argv[1]);

  if(uuid_parse((const char *)namespace, namespace_uuid) == -1) {
    sqlite3_result_error(context, "invalid uuid", -1);
    return;
  }

  uuid_t uu;

  SHA_CTX mdctx;
  unsigned char md_value[SHA_DIGEST_LENGTH];
  SHA1_Init(&mdctx);
  SHA1_Update(&mdctx, namespace_uuid, UUID_SIZE);
  SHA1_Update(&mdctx, name, strlen((const char *)name));
  SHA1_Final(md_value, &mdctx);

  SET_VARIANT(md_value);
  SET_VERSION(md_value, 5);

  memcpy(uu, md_value, UUID_SIZE);

  uuid_unparse_lower(uu, uuid_str);
  sqlite3_result_text(context, uuid_str, UUID_LENGTH, SQLITE_TRANSIENT);
}

static void uuid_nil(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  assert(argc==0);
  sqlite3_result_text(context, "00000000-0000-0000-0000-000000000000", UUID_LENGTH, SQLITE_STATIC);
}

static void uuid_ns_dns(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  assert(argc==0);
  sqlite3_result_text(context, "6ba7b810-9dad-11d1-80b4-00c04fd430c8", UUID_LENGTH, SQLITE_STATIC);
}

static void uuid_ns_oid(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  assert(argc==0);
  sqlite3_result_text(context, "6ba7b812-9dad-11d1-80b4-00c04fd430c8", UUID_LENGTH, SQLITE_STATIC);
}

static void uuid_ns_url(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  assert(argc==0);
  sqlite3_result_text(context, "6ba7b811-9dad-11d1-80b4-00c04fd430c8", UUID_LENGTH, SQLITE_STATIC);
}

static void uuid_ns_x500(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  assert(argc==0);
  sqlite3_result_text(context, "6ba7b814-9dad-11d1-80b4-00c04fd430c8", UUID_LENGTH, SQLITE_STATIC);
}

static void uuid_to_string_func(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  assert(argc==1);
  const void *uuid_bytes = sqlite3_value_blob(argv[0]);
  if(sqlite3_value_bytes(argv[0]) != UUID_SIZE) {
    sqlite3_result_error(context, "invalid uuid bytes", -1);
    return;
  }
  uuid_string_t uuid_str;
  uuid_unparse_lower(*(uuid_t *)uuid_bytes, uuid_str);
  sqlite3_result_text(context, uuid_str, UUID_LENGTH, SQLITE_TRANSIENT);
}

static void uuid_to_blob_func(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  assert(argc==1);
  uuid_t uuid;
  const unsigned char *uuid_str	= sqlite3_value_text(argv[0]);
  if(uuid_parse((const char *)uuid_str, uuid) == -1) {
    sqlite3_result_error(context, "invalid uuid", -1);
    return;
  }
  sqlite3_result_blob(context, uuid, UUID_SIZE, SQLITE_TRANSIENT);
}

/*
** Register UUID functions to database `db`.
*/
static int register_uuid_functions(sqlite3 *db) {
  typedef struct UUIDScalar {
    const char *name;
    int argc;
    int enc;
    void (*func)(sqlite3_context*, int, sqlite3_value**);
  } UUIDScalar;

  UUIDScalar scalars[] = {
    {"uuid1",          0, SQLITE_UTF8, uuid1func},
    {"uuid3",          2, SQLITE_UTF8, uuid3func},
    {"uuid4",          0, SQLITE_UTF8, uuid4func},
    {"uuid5",          2, SQLITE_UTF8, uuid5func},
    {"uuid_nil",       0, SQLITE_UTF8, uuid_nil},
    {"uuid_ns_dns",    0, SQLITE_UTF8, uuid_ns_dns},
    {"uuid_ns_oid",    0, SQLITE_UTF8, uuid_ns_oid},
    {"uuid_ns_url",    0, SQLITE_UTF8, uuid_ns_url},
    {"uuid_ns_x500",   0, SQLITE_UTF8, uuid_ns_x500},
    {"uuid_to_string", 1, SQLITE_UTF8, uuid_to_string_func},
    {"uuid_to_blob",   1, SQLITE_UTF8, uuid_to_blob_func},
  };

  int rc = SQLITE_OK;
  int i, n;

  n = (int)(sizeof(scalars)/sizeof(scalars[0]));

  for (i = 0; rc == SQLITE_OK && i < n; i++) {
    UUIDScalar *s = &scalars[i];
    rc = sqlite3_create_function(db, s->name, s->argc, s->enc, 0, s->func, 0, 0);
  }

  return rc;
}

#ifdef _WIN32
__declspec(dllexport)
#endif
int sqlite3_uuid_init(
  sqlite3 *db,
  char **pzErrMsg,
  const sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi);
  return register_uuid_functions(db);
}
