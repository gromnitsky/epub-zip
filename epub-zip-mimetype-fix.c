#include <err.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>

#include <zip.h>

void fix_index_zero(zip_t *arc) {
  zip_file_extra_field_delete(arc, 0, ZIP_EXTRA_FIELD_ALL, ZIP_FL_LOCAL);
  zip_file_extra_field_delete(arc, 0, ZIP_EXTRA_FIELD_ALL, ZIP_FL_CENTRAL);
  zip_set_file_compression(arc, 0, ZIP_CM_STORE, 0);
}

zip_source_t* mimetype_source(zip_t *arc) {
  char *buf = strdup("application/epub+zip");
  return zip_source_buffer(arc, buf, strlen(buf), 1);
}

void swap_0_with_idx(zip_t *arc, int mimetype_idx) {
  const char *displaced_name = zip_get_name(arc, 0, 0);
  warnx("swapping index 0 (%s) with %d (mimetype)",
        displaced_name, mimetype_idx);

  zip_source_t *displaced_src = zip_source_zip_create(arc, 0 /* index */,
                                                        0, 0, -1, NULL);
  zip_file_replace(arc, mimetype_idx, displaced_src, ZIP_FL_ENC_UTF_8);
  zip_file_rename(arc, 0, "566bfa62-97ff-44df-be9a-ab9d6e4b3f93", 0);
  zip_file_rename(arc, mimetype_idx, displaced_name, ZIP_FL_ENC_UTF_8);

  zip_source_t *mimetype_src = mimetype_source(arc);
  zip_file_replace(arc, 0, mimetype_src, 0);
  zip_file_rename(arc, 0, "mimetype", 0);
}

void fix(char *file, char *error) {
  zip_t *arc = zip_open(file, 0, NULL); if (!arc) {
    snprintf(error, BUFSIZ, "can't open");
    return;
  }

  int idx = zip_name_locate(arc, "mimetype", 0);

  if (-1 == idx) {
    snprintf(error, BUFSIZ, "no 'mimetype' file");
    zip_close(arc);
    return;
  }

  if (0 != idx) swap_0_with_idx(arc, idx);

  warnx("fixing index 0");
  fix_index_zero(arc);

  if (zip_close(arc) < 0) snprintf(error, BUFSIZ, zip_strerror(arc));
}

int main(int argc, char **argv) {
  if (argc < 2) errx(1, "Usage: %s file.zip", basename(argv[0]));

  char error[BUFSIZ] = "";
  fix(argv[1], error);
  if (0 != strlen(error)) errx(1, "%s: %s", argv[1], error);
}
