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

// https://stackoverflow.com/a/175787
function isNumeric(str) {
	return !isNaN(str) && !isNaN(parseFloat(str));
}
function isInteger(str) {
	return Number.isInteger(parseFloat(str));
}

const WARN = document.getElementById("warn");

const FIELD_ID = ["width", "height", "remin", "immin", "remax", "immax", "iters", "threads", "simd"];
const FIELD_NAME = ["width", "height", "min real", "min imag", "max real", "max imag", "iterations", "threads", "simd"];

document.getElementById("generate").onclick = () => {
	let inputs = new Array(9);
	for (let i = 0; i < 9; i++) {
		inputs[i] = document.getElementById(FIELD_ID[i]).value;
		if ((i >= 0 && i <= 1) || (i >= 6 && i <= 7)) {
			if (isInteger(inputs[i]) && parseInt(inputs[i]) > 0)
				inputs[i] = parseInt(inputs[i]);
			else {
				WARN.textContent = `Field "${FIELD_NAME[i]}" must be a positive integer`;
				return;
			}
		}
		if (i >= 2 && i <= 5) {
			if (isNumeric(inputs[i]))
				inputs[i] = parseInt(inputs[i]);
			else {
				WARN.textContent = `Field "${FIELD_NAME[i]}" must be numeric`;
				return;
			}
		}
	}

	if (inputs[2] >= inputs[4]) {
		WARN.textContent = `Min real must be less than max real`;
		return;
	}
	if (inputs[3] >= inputs[5]) {
		WARN.textContent = `Min imag must be less than max imag`;
		return;
	}

	console.log(inputs);
};