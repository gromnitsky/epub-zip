#include <fcntl.h>
#include <err.h>
#include <libgen.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include <archive.h>
#include <archive_entry.h>

bool mimetype_add(struct archive *arc) {
  bool r = true;
  archive_write_zip_set_compression_store(arc);
  struct archive_entry *entry = archive_entry_new();

  archive_entry_set_pathname(entry, "mimetype");
  char *contents = "application/epub+zip";
  archive_entry_set_mode(entry, AE_IFREG | 0644);
  archive_entry_set_size(entry, strlen(contents));
  archive_write_header(arc, entry);

  if (archive_write_data(arc, contents, strlen(contents)) < 0) r = false;

  archive_entry_free(entry);
  return r;
}

char *ext(char *file) {
  char *dot = strrchr(file, '.');
  if (!dot || dot == file) return "";
  return dot;
}

void set_compression(struct archive *arc, char *file) {
  char *already_compressed = ".png.jpg.gif";
  archive_write_zip_set_compression_store(arc);
  if (0 == strlen(ext(file)) || !strstr(already_compressed, ext(file)))
    archive_write_zip_set_compression_deflate(arc);
}

bool file_add(struct archive *arc, char *file) {
  bool r = true;
  struct stat st;
  if (-1 == stat(file, &st)) return false;

  set_compression(arc, file);
  struct archive_entry *entry = archive_entry_new();
  archive_entry_set_pathname(entry, file);

  if (S_ISDIR(st.st_mode)) {
    archive_entry_set_mode(entry, S_IFDIR | 0755);
    archive_entry_set_size(entry, 512);
  } else if (S_ISREG(st.st_mode)) {
    archive_entry_set_mode(entry, AE_IFREG | 0644);
    archive_entry_set_size(entry, st.st_size);
  } else {
    errno = EINVAL;
    archive_entry_free(entry);
    return false;
  }

  archive_write_header(arc, entry);

  if (S_ISDIR(st.st_mode)) {
    if (archive_write_data(arc, "12345678", 9) < 0) r = false;
  } else {
    int fd = open(file, O_RDONLY);
    char buf[st.st_size];
    int len = read(fd, buf, sizeof(buf));
    if (-1 == len) {
      r = false;
    } else {
      if (archive_write_data(arc, buf, len) < 0) r = false;
    }
    close(fd);
  }

  archive_entry_free(entry);
  return r;
}

char* zip(char *out, char **files) {
  char *file;

  struct archive *arc = archive_write_new();
  archive_write_set_format_zip(arc);
  archive_write_open_filename(arc, out);

  mimetype_add(arc);

  while (*files) {
    file = (char*)*files;
    if (!file_add(arc, file)) break;

    files++;
    file = "";
  }

  archive_write_close(arc);
  archive_write_free(arc);
  if (0 != strlen(file)) unlink(out);
  return file;
}

int main(int argc, char **argv) {
  if (argc < 3) errx(1, "Usage: %s out.zip file ...", basename((char*)argv[0]));

  argv++;
  char *out = *argv++;
  char *badfile = zip(out, argv);
  if (0 != strlen(badfile)) err(1, "%s", badfile);
}
