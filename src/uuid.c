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
#include <openssl/evp.h>
#include <uuid/uuid.h>
#include <string.h>

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

static void uuid_v3_or_v5(
  int version,
  uuid_t namespace,
  const unsigned char *name,
  uuid_t uu
){
  EVP_MD_CTX mdctx;
  const EVP_MD *md;
  unsigned char md_value[EVP_MAX_MD_SIZE];
  int md_len;

  switch(version)
  {
    case 3:
      md = EVP_get_digestbyname("MD5");
      break;
    case 5:
      md = EVP_get_digestbyname("SHA1");
      break;
  }

  EVP_MD_CTX_init(&mdctx);
  EVP_DigestInit_ex(&mdctx, md, NULL);
  EVP_DigestUpdate(&mdctx, namespace, 16);
  EVP_DigestUpdate(&mdctx, name, strlen((const char *)name));
  EVP_DigestFinal_ex(&mdctx, md_value, &md_len);
  EVP_MD_CTX_cleanup(&mdctx);

  set_variant(md_value);
  set_version(md_value, version);

  memcpy(uu, md_value, 16);
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
  uuid_t ns_uuid;
  char uuid_str[37];

  const unsigned char *ns    = sqlite3_value_text(argv[0]);
  const unsigned char *input = sqlite3_value_text(argv[1]);

  uuid_parse((const char *)ns, ns_uuid);

  uuid_t uu;
  uuid_v3_or_v5(3, ns_uuid, input, uu);
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
  uuid_t ns_uuid;
  char uuid_str[37];

  const unsigned char *ns    = sqlite3_value_text(argv[0]);
  const unsigned char *input = sqlite3_value_text(argv[1]);

  uuid_parse((const char *)ns, ns_uuid);

  uuid_t uu;
  uuid_v3_or_v5(5, ns_uuid, input, uu);
  uuid_unparse_lower(uu, uuid_str);
  sqlite3_result_text(context, uuid_str, 36, SQLITE_TRANSIENT);
}

static void uuid_ns_dns(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  char uuid_str[37];
  uuid_unparse_lower(NAMESPACE_DNS, uuid_str);
  sqlite3_result_text(context, uuid_str, 36, SQLITE_TRANSIENT);
}

static void uuid_ns_oid(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  char uuid_str[37];
  uuid_unparse_lower(NAMESPACE_OID, uuid_str);
  sqlite3_result_text(context, uuid_str, 36, SQLITE_TRANSIENT);
}

static void uuid_ns_url(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  char uuid_str[37];
  uuid_unparse_lower(NAMESPACE_URL, uuid_str);
  sqlite3_result_text(context, uuid_str, 36, SQLITE_TRANSIENT);
}

static void uuid_ns_x500(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
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
  OpenSSL_add_all_digests();
  return register_uuid_functions(db);
}
