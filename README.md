Live entropy plot from stream sources or files, by block.

```console
$ gcc -o chaoscope chaoscope.c -lm -lraylib
$ cat /dev/urandom | ./chaoscope
```

![arecord -t raw | ./chaoscope](assets/chaoscope-stdin.gif)
