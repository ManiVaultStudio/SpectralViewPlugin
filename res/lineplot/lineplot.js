// =============================================================================
// find out if the QtBridge is available
// otherwise we are running in the browser
// =============================================================================
try {
    new QWebChannel(qt.webChannelTransport, function (channel) {

        QtBridge = channel.objects.QtBridge;

        QtBridge.qt_setData.connect(function () { setData(arguments[0]); });
        //QtBridge.qt_setSelection.connect(function () { setSelection(arguments[0]); });
        //QtBridge.qt_setHighlight.connect(function () { setHighlight(arguments[0]); });
        QtBridge.qt_addAvailableData.connect(function () { addAvailableData(arguments[0]); });
        //QtBridge.qt_setMarkerSelection.connect(function () { initMarkerSelection(arguments[0]); });

        notifyBridgeAvailable();
    });
} catch (error) {
    isQtAvailable = false;
    log("could not connect qt");
}

// =============================================================================
// globals
// =============================================================================

// data ========================================================================
var _data = null;
var _spectra = [];
var _availableDataSets = [];
var maxY = 0.1;
var minY = 0;

// sizes =======================================================================
var _lineChartWidth;
var _lineChartHeight;


// UI ==========================================================================
var _margin = { top: 10, right: 80, bottom: 30, left: 60 },
    _lineChartWidth = width - _margin.left - _margin.right,
    _lineChartHeight = height - _margin.top - _margin.bottom;

var _svgSelection = d3.select("#line_chart").append("svg")
    .attr("width", _lineChartWidth + _margin.left + _margin.right)
    .attr("height", _lineChartHeight + _margin.top + _margin.bottom);

var _lineChart = _svgSelection
    .append("g")
    .attr("transform",
        "translate(" + _margin.left + "," + _margin.top + ")");

// Add X axis
var x = d3.scaleLinear()
    .domain([400, 900])
    .range([0, _lineChartWidth]);
_lineChart.append("g")
    .attr("class", "xAxis")
    .attr("transform", "translate(0," + _lineChartHeight + ")")
    .call(d3.axisBottom(x));

// X axis label
var xLabel = _lineChart.append("text")
    .attr("class", "xLabel")
    .attr("text-anchor", "end")
    .attr("x", _lineChartWidth)
    .attr("y", _lineChartHeight - 6)
    .text("Wavelength (nm)");

// Add Y axis
var y = d3.scaleLinear()
    .domain([minY, maxY])
    .range([_lineChartHeight, 0]);
_lineChart.append("g")
    .attr("class", "yAxis")
    .call(d3.axisLeft(y));

// Y axis label
var yLabel = _lineChart.append("text")
    .attr("class", "yLabel")
    .attr("text-anchor", "end")
    .attr("y", 6)
    .attr("dy", ".75em")
    .attr("transform", "rotate(-90)")
    .text("Value");

// Set confidence interval for the selection
var _stdArea = _lineChart.append("path")
    .attr("class", "stdInterval")
    .attr("fill", "black")
    .attr("stroke", "none");

// Add the line for the selection
var _selectionLine = _lineChart.append("path")
    .attr("class", "selectionLine")
    .attr("opacity", 1)
    .attr("fill", "none")
    .attr("stroke", "black")
    .attr("stroke-width", 2);

// Create the text that travels along the curve of chart
var focusText = _lineChart
    .append('g')
    .append('text')
    .style("opacity", 1)
    .attr("text-anchor", "left")
    .attr("alignment-baseline", "middle");

// Create a rect on top of the svg area: this rectangle recovers mouse position
var topRect = _lineChart
    .append('rect')
    .style("fill", "none")
    .style("pointer-events", "all")
    .attr('width', _lineChartWidth)
    .attr('height', _lineChartHeight)
    .on('mouseover', mouseover)
    .on('mousemove', mousemove)
    .on('mousedown', mousedown)
    .on('mouseup', mouseup);

var line = d3.line()
    .x(function (d) { return x(d.x); })
    .y(function (d) { return y(d.y); });

var area = d3.area()
    .x(function (d) { return x(d.x) })
    .y0(function (d) { return y(d.CI_Right) })
    .y1(function (d) { return y(d.CI_Left) });

drawRGBlines();
setRGBCheckbox();
setCICheckbox();

function drawLineChart() {

    // append the svg object to the body of the page
    _svgSelection
        .attr("width", _lineChartWidth + _margin.left + _margin.right)
        .attr("height", _lineChartHeight + _margin.top + _margin.bottom)
    
    x.range([0, _lineChartWidth]);
    _lineChart.select("g.xAxis")
        .attr("transform", "translate(0," + _lineChartHeight + ")")
        .call(d3.axisBottom(x));

    xLabel
        .attr("x", _lineChartWidth)
        .attr("y", _lineChartHeight - 6);

    y.range([_lineChartHeight, 0]);
    _lineChart.select("g.yAxis")
        .call(d3.axisLeft(y));

    topRect
        .attr('width', _lineChartWidth)
        .attr('height', _lineChartHeight);

    updateRGBlines();
}

// =============================================================================
// external
// =============================================================================
function addAvailableData(name) {

    for (var i = 0; i < _availableDataSets.length; i++) {
        if (name == _availableDataSets[i]) {
            return;
        }
    }

    _availableDataSets.push(name);
}

function setData(d) {

    log("setting data");

    _data = JSON.parse(d);
    addData();

    log("Data set.");
}

function sendRGBWavelengths() {

    if (isQtAvailable) {
        QtBridge.js_setRGBWavelengths(wavelengthR, wavelengthG, wavelengthB);
    }
}


// =============================================================================
// Windowing ===================================================================
// =============================================================================
function resize() {

    width = window.innerWidth;
    height = window.innerHeight - 200;

    if (width > _margin.left + _margin.right)
        _lineChartWidth = width - _margin.left - _margin.right;
    else
        _lineChartWidth = 50;

    if (height > _margin.top + _margin.bottom)
        _lineChartHeight = height - _margin.top - _margin.bottom;
    else
        _lineChartHeight = 50;
    
    drawLineChart();
}

resize();
d3.select(window).on("resize", resize);
