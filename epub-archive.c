#include <fcntl.h>
#include <err.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <archive.h>
#include <archive_entry.h>

void mimetype_add(struct archive *arc) {
  archive_write_zip_set_compression_store(arc);
  struct archive_entry *entry = archive_entry_new();

  archive_entry_set_pathname(entry, "mimetype");
  char *contents = "application/epub+zip";
  archive_entry_set_mode(entry, AE_IFREG | 0644);
  archive_entry_set_size(entry, strlen(contents));
  archive_write_header(arc, entry);
  archive_write_data(arc, contents, strlen(contents));

  archive_entry_free(entry);
}

char *ext(char *file) {
  char *dot = strrchr(file, '.');
  return (!dot || dot == file) ? "" : dot;
}

void set_compression(struct archive *arc, char *file) {
  char *already_compressed = ".png.jpg.gif";
  archive_write_zip_set_compression_store(arc);
  if (0 == strlen(ext(file)) || !strstr(already_compressed, ext(file)))
    archive_write_zip_set_compression_deflate(arc);
}

typedef struct { char *file, *message; } MyError;

void error_set_message(MyError *error, const char *message) {
  error->message = strdup(message);
}

void file_add(struct archive *arc, char *file, MyError *error) {
  struct stat st; if (-1 == stat(file, &st)) {
    error_set_message(error, strerror(errno));
    return;
  }

  set_compression(arc, file);
  struct archive_entry *entry = archive_entry_new();
  archive_entry_set_pathname(entry, file);

  if (S_ISDIR(st.st_mode)) {
    archive_entry_set_mode(entry, AE_IFDIR | 0755);
    archive_entry_set_size(entry, 512);
  } else if (S_ISREG(st.st_mode)) {
    archive_entry_set_mode(entry, AE_IFREG | 0644);
    archive_entry_set_size(entry, st.st_size);
  } else {
    error_set_message(error, strerror(EINVAL));
    archive_entry_free(entry);
    return;
  }

  archive_write_header(arc, entry);

  if (S_ISDIR(st.st_mode)) {
    if (archive_write_data(arc, "12345678", 9) < 0)
      error_set_message(error, archive_error_string(arc));
  } else {
    char buf[st.st_size];
    int fd = open(file, O_RDONLY);
    int len = read(fd, buf, sizeof(buf));
    if (-1 == len) {
      error_set_message(error, strerror(errno));
    } else {
      if (archive_write_data(arc, buf, len) < 0)
        error_set_message(error, archive_error_string(arc));
    }
    close(fd);
  }

  archive_entry_free(entry);
}

void zip(char *out, char **file_list, MyError *error) {
  struct archive *arc = archive_write_new();
  archive_write_set_format_zip(arc);
  archive_write_open_filename(arc, out);

  mimetype_add(arc);

  while (*file_list) {
    char *file = *file_list;
    error->file = file;
    file_add(arc, file, error); if (error->message) break;
    file_list++;
  }

  archive_write_close(arc);
  archive_write_free(arc);
  if (error->message) unlink(out);
}

int main(int argc, char **argv) {
  if (argc < 3) errx(1, "Usage: %s out.zip file ...", basename(argv[0]));

  MyError error = {};
  zip(argv[1], argv+2, &error);
  if (error.message) {
    warnx("%s: %s", error.file, error.message);
    free(error.message);
    return 1;
  }
}
