emcc base.c nosimd.c sse.c wasm.c linkedlist.c -o libmandel.js \
	-Wall -Wextra -pedantic -lm -msse2 -msimd128 -O3 \
	-sEXPORTED_FUNCTIONS=['_do_calculation_wasm','_multiply_array','_malloc','_free'] \
	-sEXPORTED_RUNTIME_METHODS=ccall,cwrap \