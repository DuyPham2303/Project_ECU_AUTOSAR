/* platform/csvlib/csv_io.h */
#ifndef CSV_IO_H
#define CSV_IO_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CSV_FILE_PATH
#define CSV_FILE_PATH "UI/data.csv"
#endif

/* Return 0 = OK */
int  csv_getInt(const char* key, int* out);
int  csv_setInt(const char* key, int value);
int  csv_getString(const char* key, char* buf, size_t buflen);
int  csv_setString(const char* key, const char* val);

#ifdef __cplusplus
}
#endif

#endif /* CSV_IO_H */
