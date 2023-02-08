#include <sys/stat.h>
#include <fcntl.h>
#include <err.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include <zip.h>

typedef struct { const char *file, *message; } MyError;

char *ext(char *file) {
  char *dot = strrchr(file, '.');
  return (!dot || dot == file) ? "" : dot;
}

void set_compression(zip_t *arc, int idx, char *file) {
  char *already_compressed = ".png.jpg.gif";
  if (0 != strlen(ext(file)) && strstr(already_compressed, ext(file)))
    zip_set_file_compression(arc, idx, ZIP_CM_STORE, 0);
}

void file_add(zip_t *arc, zip_source_t **src, int idx,
              char *file, MyError *error) {
  struct stat st; if (-1 == stat(file, &st)) {
    error->message = strerror(errno);
    return;
  }

  if (S_ISDIR(st.st_mode)) {
    zip_dir_add(arc, file, ZIP_FL_ENC_UTF_8);
  } else if (S_ISREG(st.st_mode)) {
    if (NULL == (src[idx] = zip_source_file(arc, file, 0, -1)) ||
        zip_file_add(arc, file, src[idx], ZIP_FL_ENC_UTF_8) < 0) {
      zip_source_free(src[idx]);
      error->message = zip_strerror(arc);
    } else {
      set_compression(arc, idx, file);
    }
  } else
    error->message = strerror(EINVAL);
}

void mimetype_add(zip_t *arc, zip_source_t **src) {
  char *buf = "application/epub+zip";
  src[0] = zip_source_buffer(arc, buf, strlen(buf), 0);
  zip_file_add(arc, "mimetype", src[0], ZIP_FL_ENC_UTF_8);
}

void epub(char *out, char **file_list, int file_list_len, MyError *error) {
  zip_source_t *src[++file_list_len]; // +1 for "mimetype" entry
  zip_t *arc = zip_open(out, ZIP_CREATE|ZIP_TRUNCATE, NULL);

  mimetype_add(arc, src);

  for (int idx = 1; idx < file_list_len; idx++) {
    char *file = file_list[idx-1];
    warnx("adding %s", file);
    error->file = file;
    file_add(arc, src, idx, file, error); if (error->message) break;
  }

  if (!error->message) {
    if (zip_close(arc) < 0) error->message = zip_strerror(arc);
  }
  if (error->message) {
    zip_discard(arc);
    unlink(out);
  }
}

int main(int argc, char **argv) {
  if (argc < 3) errx(1, "Usage: %s out.zip file ...", basename(argv[0]));

  MyError error = {};
  epub(argv[1], argv+2, argc-2, &error);
  if (error.message) errx(1, "%s: %s", error.file, error.message);
}
