emcc base.c nosimd.c sse.c wasm.c linkedlist.c -o libmandel.js \
	-Wall -Wextra -pedantic -lm -msse2 -msimd128 -O3 -ffast-math \
	-sEXPORTED_FUNCTIONS=['_do_calculation_wasm','_malloc','_free','_do_calculation_wasm_no_thread'] \
	-sEXPORTED_RUNTIME_METHODS=ccall,cwrap \
	-sALLOW_MEMORY_GROWTH -sASSERTIONS -s WASM=1 \
	# -pthread -sPTHREAD_POOL_SIZE_STRICT=0