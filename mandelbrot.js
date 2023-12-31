function isInteger(str) {
	return Number.isInteger(parseFloat(str));
}

const FIELDS = ["iterations", "threads"];

const frontLayer = document.getElementById("frontLayer");

var recenter = 0;
var redistance = 5;
var imcenter = 0;

var widthInput = document.getElementById("width");
var heightInput = document.getElementById("height");

var currentWidth = widthInput.value = document.documentElement.clientWidth;
var currentHeight = heightInput.value = document.documentElement.clientHeight;

var windowWidth = currentWidth;
var windowHeight = currentHeight;

widthInput.onchange = () => {
	let ratio = currentHeight / currentWidth;
	currentHeight = heightInput.value = Math.floor(ratio * widthInput.value);
	currentWidth = widthInput.value;
}

var workerIsTerminated = false;
var worker = new Worker("./worker.js");

function askForRescale(width, height) {
	let ok = confirm("Detected a window resize. Update width and height of drawing canvas? OK to accept, cancel to keep current.")
	if (!ok)
		return;
	windowWidth = currentWidth = widthInput.value = width;
	windowHeight = currentHeight = heightInput.value = height;
}

function scaleImage(image, factor, x, y) {
	let outermostDiv = image;
	while (outermostDiv.parentNode != document.body)
		outermostDiv = outermostDiv.parentNode;

	let newDiv = document.createElement("div");
	newDiv.style.transformOrigin = `${x}px ${y}px`;
	newDiv.style.transform = `scale(${factor})`;
	newDiv.appendChild(outermostDiv);
	document.body.appendChild(newDiv);
}
function translateImage(image, x, y) {
	let outermostDiv = image;
	while (outermostDiv.parentNode != document.body)
		outermostDiv = outermostDiv.parentNode;

	let newDiv = document.createElement("div");
	newDiv.style.transform = `translate(${x}px, ${y}px)`;
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

var lastRawColors = null;
var pastremin, pastimmin, pastremax, pastimmax;

function draw(usePast) {
	let start = new Date();

	if (workerIsTerminated)
		worker = new Worker("./worker.js");

	let width = widthInput.value;
	let height = heightInput.value;

	let remin = recenter - redistance;
	let remax = recenter + redistance;

	let imdistance = redistance * (height / width);

	let immin = imcenter - imdistance;
	let immax = imcenter + imdistance;

	let input = new Array(2);
	for (let i = 0; i < 2; i++) {
		input[i] = document.getElementById(FIELDS[i]).value;
		if (isInteger(input[i]) && parseInt(input[i]) > 0)
			input[i] = parseInt(input[i]);
		else {
			warn.textContent = `Field "${FIELDS[i]}" must be a positive integer`;
			return;
		}
	}

	// let ctx = canvas.getContext("2d");
	let ctx = canvas.getContext("2d", { alpha: false });
	let imgdata = ctx.getImageData(0, 0, width, height);

	ctx.canvas.width = width;
	ctx.canvas.height = height;

	worker.postMessage({
		width: width,
		height: height,
		remin: remin,
		immin: immin,
		remax: remax,
		immax: immax,
		max_iters: input[0],
		threads: input[1],
		lastRawColors: usePast ? lastRawColors : null,
		pastremin: pastremin,
		pastremax: pastremax,
		pastimmin: pastimmin,
		pastimmax: pastimmax,
	});

	worker.onmessage = e => {
		imgdata.data.set(e.data.processedColors);

		lastRawColors = e.data.rawColors;
		pastremin = remin, pastremax = remax;
		pastimmin = immin, pastimmax = immax;

		ctx.putImageData(imgdata, 0, 0);
		canvas.style.transform = "scale(1)";
		canvas.className = "";

		// https://developer.mozilla.org/en-US/docs/Web/API/HTMLCanvasElement/toDataURL
		oldCanvasImage.className = "hidden";
		clearTransforms(oldCanvasImage);
		oldCanvasImage.src = canvas.toDataURL();
		oldCanvasImage.className = "";
		
		let end = new Date();
		console.log(`total time: ${end - start}ms`);
	}
}


document.getElementById("generate").onclick = () => draw(false);
draw(false);

// https://stackoverflow.com/questions/22427395/what-is-the-google-map-zoom-algorithm
function processWheel(e) {
	worker.terminate();
	workerIsTerminated = true;
	e.preventDefault();

	let clickX = e.clientX;
	let clickY = e.clientY;

	let factor = Math.pow(0.75, e.deltaY > 0 ? -1 : 1);

	scaleImage(oldCanvasImage, 1 / factor, clickX, clickY);

	let width = document.documentElement.clientWidth;
	let height = document.documentElement.clientHeight;
	if (windowWidth != width || windowHeight != height)
		askForRescale(width, height);
	width = widthInput.value;
	height = heightInput.value;

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

	draw(false);
}

frontLayer.onwheel = e => {
	processWheel(e);
};

var dragStartX = -1;
var dragStartY = -1;

function processDrag(e) {
	worker.terminate();
	workerIsTerminated = true;
	e.preventDefault();

	let endX = e.clientX;
	let endY = e.clientY;

	translateImage(oldCanvasImage, endX - dragStartX, endY - dragStartY);

	let width = document.documentElement.clientWidth;
	let height = document.documentElement.clientHeight;
	if (windowWidth != width || windowHeight != height)
		askForRescale(width, height);
	width = widthInput.value;
	height = heightInput.value;

	let imdistance = redistance * (height / width);

	recenter -= (2 * redistance / width) * (endX - dragStartX);
	imcenter += (2 * imdistance / height) * (endY - dragStartY);
	
	dragStartX = -1, dragStartY = -1;

	draw(true);
}

frontLayer.ondragstart = e => {
	dragStartX = e.clientX;
	dragStartY = e.clientY;
};
frontLayer.ondragend = e => {
	if (dragStartX == -1 || dragStartY == -1)
		return;
	processDrag(e);
};

document.getElementById("save").onclick = () => {
	// https://stackoverflow.com/a/44487883
	var link = document.getElementById("download");
	link.setAttribute("download", "image.png");
	link.setAttribute("href", canvas.toDataURL("image/png").replace("image/png", "image/octet-stream"));
	link.click();
}

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