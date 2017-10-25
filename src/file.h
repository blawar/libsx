#ifndef FILE_H
#define FILE_H

SXNS_START

handle& fopen(const char * filename, const char * mode);
bool fclose(handle &f);
int64 filesize(handle &f);
int64 filesize(const char* filename);
bool file_exists(const char* filename);
int64 fread(handle &f, string &buf, int64 length = 0, bool append = false);
int64 fwrite(handle &f, string &buf, int64 length = 0);

int64 recv(handle s, string &buf, int64 len=0, int flags=0);
int64 send(handle s, string &buf, int flags=0);

SXNS_END

#endif
