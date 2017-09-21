# static-httpd

A fast static http server written in C.

Works under MacOS and Linux.

## Building

```
$ make
```

## Running

```
$ ./httpd -d dirname -p port
```

Defaults to `.` for `dirname`, and `8080` for `port`.
