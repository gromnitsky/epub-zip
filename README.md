~~~
$ wc -l *.c
  91 epub-zip.c
  57 epub-zip-mimetype-fix.c
 148 total
~~~

## epub-zip

Makes a correct EPUB 3 OCF ZIP Container:

* automatically adds `mimetype` file entry as a 1st file without
  compression & extra fields;
* doesn't compress already compressed files like jpegs;
* won't allow adding non-supported file types like symlinks.

Usage:

    $ epub-zip out.epub `find src`

## epub-zip-mimetype-fix

Corrects a botched EPUB 3 OCF ZIP Container in-place:

* moves `mimetype` file to the 1st position in an archive;
* removes compression & extra fileds from `mimetype` file.

Usage:

    $ epub-zip-mimetype-fix file.epub

## Compilation

~~~
$ sudo dnf install libzip-devel     # tested with 1.9.2
$ make
~~~

## License

MIT.
