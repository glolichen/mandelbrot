function red(num) { return num >> 16 & 255; }
function green(num) { return num >> 8 & 255; }
function blue(num) { return num & 255; }

const calculate_function = Module.cwrap(
	'do_calculation_wasm', 'void', ['number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number', 'number']
);

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

function isInteger(str) {
	return Number.isInteger(parseFloat(str));
}

const FIELDS = ["iterations", "threads", "simd"];
const SIMD = ["none", "sse", "wasm"];

var recenter = 0;
var redistance = 5;
var imcenter = 0;

function draw() {
	let start = new Date();

	let width = window.screen.width;
	let height = window.screen.height;

	let remin = recenter - redistance;
	let remax = recenter + redistance;

	let imdistance = (remax - remin) * (height / width) / 2;

	let immin = imcenter - imdistance;
	let immax = imcenter + imdistance;

	let input = new Array(3);
	for (let i = 0; i < 2; i++) {
		input[i] = document.getElementById(FIELDS[i]).value;
		if (isInteger(input[i]) && parseInt(input[i]) > 0)
			input[i] = parseInt(input[i]);
		else {
			warn.textContent = `Field "${FIELDS[i]}" must be a positive integer`;
			return;
		}
	}
	input[2] = document.getElementById(FIELDS[2]).value;

	let ctx = canvas.getContext("2d", { alpha: false });

	ctx.canvas.width  = width;
	ctx.canvas.height = height;

	let simd = SIMD.indexOf(input[2]);
	if (simd == -1)
		simd = 0;

	let colors = genMandelbrot(width, height, remin, immin, remax, immax, input[0], input[1], simd);

	let startDraw = new Date();

	for (let i = 0; i < width * height; i++) {
		ctx.fillStyle = `rgb(${red(colors[i])}, ${green(colors[i])}, ${blue(colors[i])})`;
		ctx.fillRect(i % width, Math.floor(i / width), 1, 1);
	}

	let finish = new Date();

	console.log(`drawing time: ${finish - startDraw}ms`);
	console.log(`total time: ${finish - start}ms`);
}

document.getElementById("generate").onclick = draw;


// // https://stackoverflow.com/a/175787
// function isNumeric(str) {
// 	return !isNaN(str) && !isNaN(parseFloat(str));
// }
// function isInteger(str) {
// 	return Number.isInteger(parseFloat(str));
// }

// const warn = document.getElementById("warn");
// const canvas = document.getElementById("canvas");

// const FIELD_ID = ["res", "remin", "immin", "remax", "immax", "iters", "threads", "simd"];
// const FIELD_NAME = ["resolution", "min real", "min imag", "max real", "max imag", "iterations", "threads", "simd"];
// const SIMD = ["none", "sse", "wasm"];

// document.getElementById("generate").onclick = () => {
// 	let input = new Array(8);
// 	for (let i = 0; i < 8; i++) {
// 		input[i] = document.getElementById(FIELD_ID[i]).value;
// 		if (i == 0 || (i >= 5 && i <= 6)) {
// 			if (isInteger(input[i]) && parseInt(input[i]) > 0)
// 				input[i] = parseInt(input[i]);
// 			else {
// 				warn.textContent = `Field "${FIELD_NAME[i]}" must be a positive integer`;
// 				return;
// 			}
// 		}
// 		if (i >= 1 && i <= 4) {
// 			if (isNumeric(input[i]))
// 				input[i] = parseFloat(input[i]);
// 			else {
// 				warn.textContent = `Field "${FIELD_NAME[i]}" must be numeric`;
// 				return;
// 			}
// 		}
// 	}

// 	if (input[1] >= input[3]) {
// 		warn.textContent = `Min real must be less than max real`;
// 		return;
// 	}
// 	if (input[2] >= input[4]) {
// 		warn.textContent = `Min imag must be less than max imag`;
// 		return;
// 	}

// 	let start = new Date();

// 	warn.textContent = "";

// 	let width, height;

// 	let wthRatio = (input[3] - input[1]) / (input[4] - input[2]);
// 	// let wthRatio = (remax - remin) / (immax - immin);
// 	if (wthRatio >= 1) {
// 		width = Math.floor(input[0]);
// 		height = Math.floor(input[0] / wthRatio);
// 	}
// 	else {
// 		height = Math.floor(input[0]);
// 		width = Math.floor(input[0] * wthRatio);
// 	}

// 	let ctx = canvas.getContext("2d");

// 	ctx.canvas.width  = width;
// 	ctx.canvas.height = height;

// 	let simd = SIMD.indexOf(input[7]);
// 	if (simd == -1)
// 		simd = 0;

// 	let colors = genMandelbrot(width, height, input[1], input[2], input[3], input[4], input[5], input[6], simd);

// 	let startDraw = new Date();

// 	for (let i = 0; i < width * height; i++) {
// 		ctx.fillStyle = `rgba(${red(colors[i])}, ${green(colors[i])}, ${blue(colors[i])}, 255)`;
// 		ctx.fillRect(i % width, Math.floor(i / width), 1, 1);
// 	}

// 	let finish = new Date();

// 	console.log(`drawing time: ${finish - startDraw}ms`);
// 	console.log(`total time: ${finish - start}ms`);
// };

// https://www.w3schools.com/howto/howto_js_draggable.asp
dragElement(document.getElementById("options"));
function dragElement(elmnt) {
	var pos1 = 0, pos2 = 0, pos3 = 0, pos4 = 0;
	if (document.getElementById(elmnt.id + "header")) {
		// if present, the header is where you move the DIV from:
		document.getElementById(elmnt.id + "header").onmousedown = dragMouseDown;
	} else {
		// otherwise, move the DIV from anywhere inside the DIV:
		elmnt.onmousedown = dragMouseDown;
	}

	function dragMouseDown(e) {
		e = e || window.event;
		e.preventDefault();
		// get the mouse cursor position at startup:
		pos3 = e.clientX;
		pos4 = e.clientY;
		document.onmouseup = closeDragElement;
		// call a function whenever the cursor moves:
		document.onmousemove = elementDrag;
	}

	function elementDrag(e) {
		e = e || window.event;
		e.preventDefault();
		// calculate the new cursor position:
		pos1 = pos3 - e.clientX;
		pos2 = pos4 - e.clientY;
		pos3 = e.clientX;
		pos4 = e.clientY;
		// set the element's new position:
		elmnt.style.top = (elmnt.offsetTop - pos2) + "px";
		elmnt.style.left = (elmnt.offsetLeft - pos1) + "px";
	}

	function closeDragElement() {
		// stop moving when mouse button is released:
		document.onmouseup = null;
		document.onmousemove = null;
	}
}