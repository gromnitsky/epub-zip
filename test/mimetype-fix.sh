#!/bin/sh
# shellcheck disable=3043

setUp() {
    zip -rq _out/1.zip test/data
    local dir="`pwd`/_out"
    (cd test/data; zip -rq "$dir/2.epub" .)
}

tearDown() {
    rm -f _out/1.zip _out/2.epub
}

test_swap() {
    local r="`_out/epub-zip-mimetype-fix _out/2.epub 2>&1`"
    assertEquals 'epub-zip-mimetype-fix: swapping 0 (35478.png) with 4 (mimetype)
epub-zip-mimetype-fix: fixing index 0' "$r"

    r="`zipinfo _out/2.epub | grep ^- | awk '{print $5, $6, $9}' `"
    assertEquals 't- stor mimetype
tx defN dir/file1
tx defN dir/file2.p
bx defN 35478.png' "$r"

    local r="`_out/epub-zip-mimetype-fix _out/2.epub 2>&1`"
    assertEquals 'epub-zip-mimetype-fix: fixing index 0' "$r"
}

test_not_found() {
    local r="`_out/epub-zip-mimetype-fix foo.txt 2>&1`"
    assertEquals "epub-zip-mimetype-fix: foo.txt: can't open" "$r"
}

test_invalid_input() {
    local r="`_out/epub-zip-mimetype-fix /dev/null 2>&1`"
    assertEquals "epub-zip-mimetype-fix: /dev/null: can't open" "$r"

    r="`_out/epub-zip-mimetype-fix _out/1.zip 2>&1`"
    assertEquals "epub-zip-mimetype-fix: _out/1.zip: no mimetype file" "$r"
}

. /usr/share/shunit2/shunit2
