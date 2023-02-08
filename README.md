Makes a correct OCF ZIP Container for epub v3:

* automatically adds `mimetype` file entry as a 1st file without
  compression & extra fields;
* doesn't compress already compressed files like jpegs;
* won't allow adding non-supported file types like symlinks;
* only 85 lines of plain C.

## Usage

    $ epub-zip out.epub `find src`

## Compilation

~~~
$ sudo dnf install libzip-devel     # tested with 1.9.2
$ make
~~~

## License

MIT.
