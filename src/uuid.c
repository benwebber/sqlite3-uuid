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
    {"uuid1", 0, SQLITE_UTF8, uuid1func},
    {"uuid4", 0, SQLITE_UTF8, uuid4func},
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
