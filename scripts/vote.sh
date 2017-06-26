#!/bin/sh

cp -f /music/empty.m3u /music/metafile.m3u
/usr/local/bin/fxaudio /music/metafile.m3u &
/usr/local/bin/voted &
/usr/local/bin/listmaker /usr/local/voted/voted.results /music/metafile.m3u &
