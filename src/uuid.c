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

const uuid_t NAMESPACE_DNS  = {0x6b,0xa7,0xb8,0x10,0x9d,0xad,0x11,0xd1,0x80,0xb4,0x00,0xc0,0x4f,0xd4,0x30,0xc8};
const uuid_t NAMESPACE_OID  = {0x6b,0xa7,0xb8,0x12,0x9d,0xad,0x11,0xd1,0x80,0xb4,0x00,0xc0,0x4f,0xd4,0x30,0xc8};
const uuid_t NAMESPACE_URL  = {0x6b,0xa7,0xb8,0x11,0x9d,0xad,0x11,0xd1,0x80,0xb4,0x00,0xc0,0x4f,0xd4,0x30,0xc8};
const uuid_t NAMESPACE_X500 = {0x6b,0xa7,0xb8,0x14,0x9d,0xad,0x11,0xd1,0x80,0xb4,0x00,0xc0,0x4f,0xd4,0x30,0xc8};


static void set_variant(
  uuid_t uu
){
  uu[8] = (uu[8] & 0xbf) | 0x80;
}

static void set_version(
  uuid_t uu,
  int version
){
  uu[6] = (uu[6] & 0x0f) | (version << 4);
}

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
  sqlite3_result_text(context, uuid_str, 36, SQLITE_TRANSIENT);
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

  if(uuid_parse((const char *)namespace, namespace_uuid) == -1)
    sqlite3_result_error(context, "invalid uuid", SQLITE_ERROR);

  uuid_t uu;

  MD5_CTX mdctx;
  unsigned char md_value[MD5_DIGEST_LENGTH];
  MD5_Init(&mdctx);
  MD5_Update(&mdctx, namespace_uuid, 16);
  MD5_Update(&mdctx, name, strlen((const char *)name));
  MD5_Final(md_value, &mdctx);

  set_variant(md_value);
  set_version(md_value, 3);

  memcpy(uu, md_value, 16);

  uuid_unparse_lower(uu, uuid_str);
  sqlite3_result_text(context, uuid_str, 36, SQLITE_TRANSIENT);
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
  sqlite3_result_text(context, uuid_str, 36, SQLITE_TRANSIENT);
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

  if(uuid_parse((const char *)namespace, namespace_uuid) == -1)
    sqlite3_result_error(context, "invalid uuid", SQLITE_ERROR);

  uuid_t uu;

  SHA_CTX mdctx;
  unsigned char md_value[SHA_DIGEST_LENGTH];
  SHA1_Init(&mdctx);
  SHA1_Update(&mdctx, namespace_uuid, 16);
  SHA1_Update(&mdctx, name, strlen((const char *)name));
  SHA1_Final(md_value, &mdctx);

  set_variant(md_value);
  set_version(md_value, 5);

  memcpy(uu, md_value, 16);

  uuid_unparse_lower(uu, uuid_str);
  sqlite3_result_text(context, uuid_str, 36, SQLITE_TRANSIENT);
}

static void uuid_nil(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  assert(argc==0);
  uuid_t uu = {0};
  char uuid_str[37];
  uuid_unparse_lower(uu, uuid_str);
  sqlite3_result_text(context, uuid_str, 36, SQLITE_TRANSIENT);
}

static void uuid_ns_dns(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  assert(argc==0);
  char uuid_str[37];
  uuid_unparse_lower(NAMESPACE_DNS, uuid_str);
  sqlite3_result_text(context, uuid_str, 36, SQLITE_TRANSIENT);
}

static void uuid_ns_oid(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  assert(argc==0);
  char uuid_str[37];
  uuid_unparse_lower(NAMESPACE_OID, uuid_str);
  sqlite3_result_text(context, uuid_str, 36, SQLITE_TRANSIENT);
}

static void uuid_ns_url(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  assert(argc==0);
  char uuid_str[37];
  uuid_unparse_lower(NAMESPACE_URL, uuid_str);
  sqlite3_result_text(context, uuid_str, 36, SQLITE_TRANSIENT);
}

static void uuid_ns_x500(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  assert(argc==0);
  char uuid_str[37];
  uuid_unparse_lower(NAMESPACE_X500, uuid_str);
  sqlite3_result_text(context, uuid_str, 36, SQLITE_TRANSIENT);
}

/*
** Register UUID functions to database `db`.
*/
int register_uuid_functions(sqlite3 *db) {
  typedef struct UUIDScalar {
    const char *name;
    int argc;
    int enc;
    void (*func)(sqlite3_context*, int, sqlite3_value**);
  } UUIDScalar;

  UUIDScalar scalars[] = {
    {"uuid1",        0, SQLITE_UTF8, uuid1func},
    {"uuid3",        2, SQLITE_UTF8, uuid3func},
    {"uuid4",        0, SQLITE_UTF8, uuid4func},
    {"uuid5",        2, SQLITE_UTF8, uuid5func},
    {"uuid_nil",     0, SQLITE_UTF8, uuid_nil},
    {"uuid_ns_dns",  0, SQLITE_UTF8, uuid_ns_dns},
    {"uuid_ns_oid",  0, SQLITE_UTF8, uuid_ns_oid},
    {"uuid_ns_url",  0, SQLITE_UTF8, uuid_ns_url},
    {"uuid_ns_x500", 0, SQLITE_UTF8, uuid_ns_x500},
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
