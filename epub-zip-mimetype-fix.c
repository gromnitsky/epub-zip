#include <err.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <zip.h>

void fix_index_zero(zip_t *arc) {
  zip_file_extra_field_delete(arc, 0, ZIP_EXTRA_FIELD_ALL, ZIP_FL_LOCAL);
  zip_file_extra_field_delete(arc, 0, ZIP_EXTRA_FIELD_ALL, ZIP_FL_CENTRAL);
  zip_set_file_compression(arc, 0, ZIP_CM_STORE, 0);
}

void swap(zip_t *arc, int a, int b) {
  const char *a_name = zip_get_name(arc, a, 0);
  zip_source_t *a_src = zip_source_zip_create(arc, a, 0, 0, -1, NULL);
  const char *b_name = zip_get_name(arc, b, 0);
  zip_source_t *b_src = zip_source_zip_create(arc, b, 0, 0, -1, NULL);
  if (getenv("EPUB_ZIP_DEBUG"))
    warnx("swapping %d (%s) with %d (%s)", a, a_name, b, b_name);

  zip_file_replace(arc, a, b_src, ZIP_FL_ENC_UTF_8);
  zip_file_replace(arc, b, a_src, ZIP_FL_ENC_UTF_8);
  zip_file_rename(arc, a, "566bfa62-97ff-44df-be9a-ab9d6e4b3f93", 0); // tmp

  zip_file_rename(arc, b, a_name, 0);
  zip_file_rename(arc, a, b_name, 0);
}

void fix(char *file, char *error) {
  zip_t *arc = zip_open(file, 0, NULL); if (!arc) {
    snprintf(error, BUFSIZ, "can't open");
    return;
  }

  int idx = zip_name_locate(arc, "mimetype", 0);

  if (-1 == idx) {
    snprintf(error, BUFSIZ, "no mimetype file");
    zip_close(arc);
    return;
  }

  if (0 != idx) swap(arc, 0, idx);

  if (getenv("EPUB_ZIP_DEBUG")) warnx("fixing index 0");
  fix_index_zero(arc);

  if (zip_close(arc) < 0) snprintf(error, BUFSIZ, zip_strerror(arc));
}

int main(int argc, char **argv) {
  if (argc < 2) errx(1, "Usage: %s file.zip", basename(argv[0]));

  char error[BUFSIZ] = "";
  fix(argv[1], error);
  if (0 != strlen(error)) errx(1, "%s: %s", argv[1], error);
}
