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
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

/*
** A UUID represents an RFC 4122 UUID.
*/
typedef struct UUID {
  unsigned char bytes[16];
} UUID;

/*
** Set the UUID version.
*/
void set_uuid_version(UUID* u, char version) {
  u->bytes[6] = (u->bytes[6] & 0x0f) | (version << 4);
}

/*
** Set the UUID variant to match the RFC4122 specification.
**
** Example:
**
**   e462c464-d4d9-433b-b372-cc90939dbe6d
**                      ^
**                      one of {8, 9, a, b}
*/
void set_uuid_variant(UUID* u) {
  u->bytes[8] = (u->bytes[8] & 0x3f) | 0x80;
}

/*
** Represent a UUID in its canonical string form.
*/
char* uuid_to_string(UUID* u) {

  int i;
  char* buf = sqlite3_malloc(sizeof(char)*36+1);

  for (i = 0; i < 4; i++) {
    sprintf(&buf[2*i], "%02x", u->bytes[i]);
  }
  sprintf(&buf[8], "-");
  for (i = 4; i < 6; i++) {
    sprintf(&buf[2*i + 1], "%02x", u->bytes[i]);
  }
  sprintf(&buf[13], "-");
  for (i = 6; i < 8; i++) {
    sprintf(&buf[2*i + 2], "%02x", u->bytes[i]);
  }
  sprintf(&buf[18], "-");
  for (i = 8; i < 10; i++) {
    sprintf(&buf[2*i + 3], "%02x", u->bytes[i]);
  }
  sprintf(&buf[23], "-");
  for (i = 10; i < 16; i++) {
    sprintf(&buf[2*i + 4], "%02x", u->bytes[i]);
  }

  return buf;
}

UUID random_uuid() {
  UUID u;

  int fd = open("/dev/urandom", O_RDONLY);
  read(fd, u.bytes, sizeof(u.bytes));
  close(fd);

  set_uuid_version(&u, 4);
  set_uuid_variant(&u);
  return u;
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
  UUID uuid = random_uuid();
  char* buf = uuid_to_string(&uuid);
  sqlite3_result_text(context, (char*)buf, 36, SQLITE_TRANSIENT);
  sqlite3_free(buf);
}

#ifdef _WIN32
__declspec(dllexport)
#endif
int sqlite3_uuid_init(
  sqlite3 *db,
  char **pzErrMsg,
  const sqlite3_api_routines *pApi
){
  int rc = SQLITE_OK;
  SQLITE_EXTENSION_INIT2(pApi);
  rc = sqlite3_create_function(db, "uuid4", 0, SQLITE_UTF8, 0, uuid4func, 0, 0);
  return rc;
}
