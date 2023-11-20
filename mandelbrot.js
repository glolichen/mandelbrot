function getRed(num) { return num >> 16 & 255; }
function getGreen(num) { return num >> 8 & 255; }
function getBlue(num) { return num & 255; }

function genMandelbrot(width, height, remin, immin, remax, immax, max_iters, thread_count, instructions) {
	calculate = Module.cwrap(
		'do_calculation_wasm', 'void', ['number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number']
	);

	// Create example data to test float_multiply_array
	let data = new Int32Array(new Array(width * height));

	// Get data byte size, allocate memory on Emscripten heap, and get pointer
	let nDataBytes = data.length * data.BYTES_PER_ELEMENT;
	let dataPtr = Module._malloc(nDataBytes);

	// Copy data to Emscripten heap (directly accessed from Module.HEAPU8)
	let dataHeap = new Uint8Array(Module.HEAPU8.buffer, dataPtr, nDataBytes);
	dataHeap.set(new Uint8Array(data.buffer));

	// Call function and get result
	calculate(dataHeap.byteOffset, width, height, remin, immin, remax, immax, max_iters, thread_count, instructions);
	let result = new Int32Array(dataHeap.buffer, dataHeap.byteOffset, data.length);

	// Free memory
	Module._free(dataHeap.byteOffset);

	return result;
}