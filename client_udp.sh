gcc -g -o i3_c i3_c.c
./i3_c 127.0.0.1 50000 | play -t raw -b 16 -c 1 -e s -r 44100 -
