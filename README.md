# Gregor
A parallel programming framework

# Compile Test Case
compile testcase: gcc -m32 -std=gnu99 -ffixed-ebx -o test_case test_case.c libgregor.a -lpthread

# Compile Test File
gcc -g -m32 heat-serial.c -o heat -lm
