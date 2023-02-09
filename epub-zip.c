#include <sys/stat.h>
#include <err.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include <zip.h>

typedef struct { char *file; char message[256]; } MyError;
void myerror_set(MyError *e, char *file, const char *msg) {
  e->file = file;
  snprintf(e->message, 256, msg);
}

typedef struct {
  zip_t *arc;
  zip_source_t **files;
  int idx;
} MyZip;

char *ext(char *file) {
  char *dot = strrchr(file, '.');
  return (!dot || dot == file) ? "" : dot;
}

void set_compression(MyZip *mz, char *file) {
  char *heystack = ".png.jpg.gif.", *e = ext(file), *p = strstr(heystack, e);
  if (0 != strlen(e) && p && '.' == (p+strlen(e))[0])
    zip_set_file_compression(mz->arc, mz->idx, ZIP_CM_STORE, 0);
}

void file_add(MyZip *mz, char *file, MyError *error) {
  struct stat st; if (-1 == stat(file, &st)) {
    myerror_set(error, file, strerror(errno));
    return;
  }

  if (S_ISREG(st.st_mode)) {
    if (NULL == (mz->files[mz->idx] = zip_source_file(mz->arc, file, 0, -1)) ||
        zip_file_add(mz->arc, file, mz->files[mz->idx], ZIP_FL_ENC_UTF_8) < 0) {
      zip_source_free(mz->files[mz->idx]);
      myerror_set(error, file, zip_strerror(mz->arc));
      return;
    }
    set_compression(mz, file);
    mz->idx++;
  } else if (!S_ISDIR(st.st_mode)) myerror_set(error, file, strerror(EINVAL));
}

void mimetype_add(MyZip *mz) {
  char *buf = strdup("application/epub+zip");
  mz->files[mz->idx] = zip_source_buffer(mz->arc, buf, strlen(buf), 1);
  zip_file_add(mz->arc, "mimetype", mz->files[mz->idx++], ZIP_FL_ENC_UTF_8);
}

void epub(char *out, char **file_list, int file_list_len, MyError *error) {
  MyZip mz = {
    .arc = zip_open(out, ZIP_CREATE|ZIP_TRUNCATE, NULL),
    .files = malloc((file_list_len + 1)*sizeof(zip_source_t*))
  };
  if (!mz.arc) {
    myerror_set(error, out, "failure to create");
    return;
  }

  mimetype_add(&mz);

  for (char **file = file_list; *file; file++) {
    if (getenv("EPUB_ZIP_DEBUG")) warnx("queueing %s", *file);
    file_add(&mz, *file, error); if (error->file) break;
  }

  if (!error->file) {           /* commit */
    if (zip_close(mz.arc) < 0) myerror_set(error, "zip", zip_strerror(mz.arc));
  }
  if (error->file) {
    zip_discard(mz.arc);
    if (mz.idx > 1) unlink(out);
  }
  free(mz.files);
}

int main(int argc, char **argv) {
  if (argc < 3) errx(1, "Usage: %s out.zip file ...", basename(argv[0]));

  MyError error = {};
  epub(argv[1], argv+2, argc-2, &error);
  if (error.file) errx(1, "%s: %s", error.file, error.message);
}
