function red(num) { return num >> 16 & 255; }
function green(num) { return num >> 8 & 255; }
function blue(num) { return num & 255; }

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

const warn = document.getElementById("warn");
const canvas = document.getElementById("canvas");

const FIELD_ID = ["res", "remin", "immin", "remax", "immax", "iters", "threads", "simd"];
const FIELD_NAME = ["resolution", "min real", "min imag", "max real", "max imag", "iterations", "threads", "simd"];
const SIMD = ["none", "sse", "avx"];

document.getElementById("generate").onclick = () => {
	let input = new Array(8);
	for (let i = 0; i < 8; i++) {
		input[i] = document.getElementById(FIELD_ID[i]).value;
		if (i == 0 || (i >= 5 && i <= 6)) {
			if (isInteger(input[i]) && parseInt(input[i]) > 0)
				input[i] = parseInt(input[i]);
			else {
				warn.textContent = `Field "${FIELD_NAME[i]}" must be a positive integer`;
				return;
			}
		}
		if (i >= 1 && i <= 4) {
			if (isNumeric(input[i]))
				input[i] = parseFloat(input[i]);
			else {
				warn.textContent = `Field "${FIELD_NAME[i]}" must be numeric`;
				return;
			}
		}
	}

	if (input[1] >= input[3]) {
		warn.textContent = `Min real must be less than max real`;
		return;
	}
	if (input[2] >= input[4]) {
		warn.textContent = `Min imag must be less than max imag`;
		return;
	}

	warn.textContent = "";

	let width, height;

	let wthRatio = (input[3] - input[1]) / (input[4] - input[2]);
	// let wthRatio = (remax - remin) / (immax - immin);
	if (wthRatio >= 1) {
		width = Math.floor(input[0]);
		height = Math.floor(input[0] / wthRatio);
	}
	else {
		height = Math.floor(input[0]);
		width = Math.floor(input[0] * wthRatio);
	}

	let ctx = canvas.getContext("2d");

	ctx.canvas.width  = width;
	ctx.canvas.height = height;

	let simd = SIMD.indexOf(input[7]);
	if (simd == -1)
		simd = 0;

	let colors = genMandelbrot(width, height, input[1], input[2], input[3], input[4], input[5], input[6], simd);
	for (let i = 0; i < width * height; i++) {
		ctx.fillStyle = `rgba(${red(colors[i])}, ${green(colors[i])}, ${blue(colors[i])}, 255)`;
		ctx.fillRect(i % width, Math.floor(i / width), 1, 1);
	}
};