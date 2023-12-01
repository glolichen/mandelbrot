function red(num) { return num >> 16 & 255; }
function green(num) { return num >> 8 & 255; }
function blue(num) { return num & 255; }

importScripts("./libmandel.js");

const calculate_function = Module.cwrap(
	'do_calculation_wasm_no_thread', 'void', ['number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number']
);
// const calculate_function = Module.cwrap(
// 	'do_calculation_wasm', 'void', ['number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number']
// );

function genMandelbrot(width, height, remin, immin, remax, immax, max_iters, thread_count, instructions) {
	let data = new Int32Array(new Array(width * height));
	let nDataBytes = data.length * data.BYTES_PER_ELEMENT;
	let dataPtr = Module._malloc(nDataBytes);
	let dataHeap = new Uint8Array(Module.HEAPU8.buffer, dataPtr, nDataBytes);
	dataHeap.set(new Uint8Array(data.buffer));

	calculate_function(dataHeap.byteOffset, width, height, remin, immin, remax, immax, max_iters, thread_count, instructions);
	dataHeap = new Uint8Array(Module.HEAPU8.buffer, dataPtr, nDataBytes);
	let result = new Int32Array(dataHeap.buffer, dataHeap.byteOffset, data.length);

	Module._free(dataHeap.byteOffset);
	return result;
}

var initialized = false;
Module['onRuntimeInitialized'] = () => initialized = true;

function waitOrDoStuff(e) {
	if (!initialized)
		setTimeout(() => waitOrDoStuff(e), 10);
	else {
		console.log("begin doing stuff");
		let rawColors = genMandelbrot(e.data.width, e.data.height, e.data.remin, e.data.immin, e.data.remax, 
			e.data.immax, e.data.max_iters, e.data.threads, e.data.simd);
		let processedColors = new Uint8ClampedArray(e.data.width * e.data.height * 4);

		for (let i = 0; i < e.data.width * e.data.height; i++) {
			let color = rawColors[i];
			processedColors[i * 4] = red(color);
			processedColors[i * 4 + 1] = green(color);
			processedColors[i * 4 + 2] = blue(color);
			processedColors[i * 4 + 3] = 255;
		}

		postMessage({
			rawColors: rawColors,
			processedColors: processedColors,
		});
	}
}

onmessage = e => {
	waitOrDoStuff(e);
}