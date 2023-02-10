#!/bin/sh
# shellcheck disable=3043

setUp() {
    _out/epub-zip _out/1.zip epub-zip.c `find test/data`
}

tearDown() {
    rm -f _out/1.zip
}

test_dirs_are_ignored() {
    local r="`zipinfo _out/1.zip | grep ^- | awk '{print $5, $6, $9}' `"
    assertEquals 'b- stor mimetype
b- defX epub-zip.c
b- stor test/data/35478.png
b- defX test/data/dir/file1
b- defX test/data/dir/file2.p
b- stor test/data/mimetype' "$r"

}

test_invalid_out() {
    local r="`_out/epub-zip /dev/null foo.txt 2>&1`"
    assertEquals 'epub-zip: /dev/null: failure to create' "$r"
}

test_invalid_out_permission_denied() {
    local r="`_out/epub-zip /1.zip epub-zip.c 2>&1`"
    assertEquals 'epub-zip: zip: Failure to create temporary file: Permission denied' "$r"
}

test_invalid_input_out_is_preserved() {
    local r="`_out/epub-zip _out/1.zip foo.txt 2>&1`"
    assertEquals 'epub-zip: foo.txt: No such file or directory' "$r"
    assertTrue "[ -f _out/1.zip ]"
}

test_invalid_input_out_is_removed() {
    local r="`_out/epub-zip _out/1.zip epub-zip.c foo.txt 2>&1`"
    assertEquals 'epub-zip: foo.txt: No such file or directory' "$r"
    assertTrue "[ ! -f _out/1.zip ]"
}

test_invalid_input() {
    local r="`_out/epub-zip _out/1.zip epub-zip.c /dev/null 2>&1`"
    assertEquals 'epub-zip: /dev/null: Invalid argument' "$r"
}

test_invalid_permission_denied() {
    local r="`_out/epub-zip _out/1.zip epub-zip.c /root/foo.txt 2>&1`"
    assertEquals 'epub-zip: /root/foo.txt: Permission denied' "$r"
}

. /usr/share/shunit2/shunit2
