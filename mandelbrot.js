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

	let imdistance = redistance * (height / width);

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

	ctx.clearRect(0, 0, canvas.width, canvas.height);

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
Module['onRuntimeInitialized'] = draw;

var isWaiting = false, isProcessing = false;

// https://stackoverflow.com/questions/22427395/what-is-the-google-map-zoom-algorithm
addEventListener("wheel", (event) => {
	if (isProcessing)
		return;

	let width = window.screen.width;
	let height = window.screen.height;

	if (event.deltaY > 0) {

	}
	if (event.deltaY < 0) {
		// zoom in
		// console.log(event.deltaY, event.clientX, event.clientY);
		let clickX = event.clientX;
		let clickY = event.clientY;
	
		let imdistance = redistance * (height / width);
	
		let remin = recenter - redistance;
		let immax = imcenter + imdistance;

		let xOnImage = remin + (2 * redistance / width) * clickX;
		let yOnImage = immax - (2 * imdistance / height) * clickY;

		let newMapWidth = 2 * 0.75 * redistance;//lets say that zoomFactor = <1.0, maxZoomFactor>
		let newMapHeight = 2 * 0.75 * imdistance;
		
		let left = xOnImage - (clickX - 0) * (newMapWidth / width);
		let top = yOnImage - (height - clickY) * (newMapHeight / height);
		let right = left + newMapWidth;
		let bottom = top + newMapHeight;

		recenter = (right + left) / 2;
		redistance = (right - left) / 2;
		imcenter = (bottom + top) / 2;

		console.log("zoom");
	}
	if (!isWaiting) {
		isWaiting = true;
		setTimeout(() => {
			isProcessing = true;
			draw();
			isProcessing = false, isWaiting = false;
		}, 500);
	}
});

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