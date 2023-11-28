function isInteger(str) {
	return Number.isInteger(parseFloat(str));
}

const FIELDS = ["iterations", "threads", "simd"];
const SIMD = ["none", "sse", "wasm"];

var recenter = 0;
var redistance = 5;
var imcenter = 0;

var workerIsTerminated = false;
var worker = new Worker("./worker.js");

function scaleAgain(image, factor, x, y) {
	let outermostDiv = image;
	while (outermostDiv.parentNode != document.body)
		outermostDiv = outermostDiv.parentNode;

	let newDiv = document.createElement("div");
	newDiv.style.transformOrigin = `${x}px ${y}px`;
	newDiv.style.transform = `scale(${factor})`;
	newDiv.appendChild(outermostDiv);
	document.body.appendChild(newDiv);
}

function clearTransforms(image) {
	let curDiv = image.parentNode;
	document.body.appendChild(image);

	let parentOfCur;
	while (curDiv != null && curDiv != document.body) {
		parentOfCur = curDiv.parentNode;
		parentOfCur.removeChild(curDiv);
		curDiv = parentOfCur;
	}
}

function draw() {
	if (workerIsTerminated)
		worker = new Worker("./worker.js");

	let width = document.documentElement.clientWidth;
	let height = document.documentElement.clientHeight;

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

	let ctx = canvas.getContext("2d");
	// let ctx = canvas.getContext("2d", { alpha: false });

	let imgdata = ctx.getImageData(0, 0, width, height);
	let imgdatalen = imgdata.data.length;

	ctx.canvas.width = width;
	ctx.canvas.height = height;

	let simd = SIMD.indexOf(input[2]);
	if (simd == -1)
		simd = 0;

	worker.postMessage({
		width: width,
		height: height,
		remin: remin,
		immin: immin,
		remax: remax,
		immax: immax,
		max_iters: input[0],
		threads: input[1],
		simd: simd,
	});

	worker.onmessage = e => {
		for (var i = 0; i < imgdatalen / 4; i++) {
			imgdata.data[4 * i] = e.data[i].red;
			imgdata.data[4 * i + 1] = e.data[i].green;
			imgdata.data[4 * i + 2] = e.data[i].blue;
			imgdata.data[4 * i + 3] = 255;
		}
		ctx.putImageData(imgdata, 0, 0);
		canvas.style.transform = "scale(1)";
		canvas.className = "";
		oldCanvasImage.className = "hidden";
		clearTransforms(oldCanvasImage);
		// https://developer.mozilla.org/en-US/docs/Web/API/HTMLCanvasElement/toDataURL
		oldCanvasImage.src = canvas.toDataURL();
		// this is weird - if there is no timeout here, there would be a very short period of time
		// where the oldCanvasImage is not rendered and the canvas is hidden, creating a white screen
		// adding this delay to hide the canvas fixes this issue
		oldCanvasImage.className = "";
		setTimeout(() => {
			canvas.className = "hidden";
		}, 100);
	}
}


document.getElementById("generate").onclick = draw;
draw();

// https://stackoverflow.com/questions/22427395/what-is-the-google-map-zoom-algorithm
function processWheel(e) {
	worker.terminate();
	workerIsTerminated = true;
	e.preventDefault();

	let clickX = e.clientX;
	let clickY = e.clientY;

	let factor = Math.pow(0.5, e.deltaY > 0 ? -1 : 1);

	scaleAgain(oldCanvasImage, 1 / factor, clickX, clickY);

	let width = document.documentElement.clientWidth;
	let height = document.documentElement.clientHeight;

	let imdistance = redistance * (height / width);

	let remin = recenter - redistance;
	let immax = imcenter + imdistance;

	let xOnImage = remin + (2 * redistance / width) * clickX;
	let yOnImage = immax - (2 * imdistance / height) * clickY;

	let newMapWidth = 2 * factor * redistance;
	let newMapHeight = 2 * factor * imdistance;
	
	let left = xOnImage - (clickX) * (newMapWidth / width);
	let top = yOnImage - (height - clickY) * (newMapHeight / height);
	let right = left + newMapWidth;
	let bottom = top + newMapHeight;

	recenter = (right + left) / 2;
	redistance = (right - left) / 2;
	imcenter = (bottom + top) / 2;
	console.log("zoom");

	draw();
}

function processDrag() {

}

oldCanvasImage.addEventListener("wheel", e => processWheel(e));

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