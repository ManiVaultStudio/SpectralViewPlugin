// =============================================================================
// find out if the QtBridge is available
// otherwise we are running in the browser
// =============================================================================
try {
    new QWebChannel(qt.webChannelTransport, function (channel) {

        QtBridge = channel.objects.QtBridge;

        QtBridge.qt_setData.connect(function () { setData(arguments[0]); });
        QtBridge.qt_setEndmemberColor.connect(function () { setEndmemberColor(arguments[0], arguments[1], arguments[2], arguments[3]); });
        QtBridge.qt_setEndmember.connect(function () { setEndmember(arguments[0]); });
        QtBridge.qt_setEndmemberRemoved.connect(function () { setEndmemberRemoved(arguments[0]); });
        QtBridge.qt_setHighlightSelection.connect(function () { setHighlightSelection(arguments[0]); });
        QtBridge.qt_enableRGBWavelengths.connect(function () { enableRGBLines(arguments[0]); });
        QtBridge.qt_enableStdArea.connect(function () { enableStdArea(arguments[0]); });
        QtBridge.qt_addAvailableData.connect(function () { addAvailableData(arguments[0]); });
        QtBridge.qt_setEndmemberVisibility.connect(function () { setEndmemberVisibility(arguments[0], arguments[1]); });


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
var _endmembers = [];
var _availableDataSets = [];
var maxY = 0.1;
var minY = 0;
var maxX = 900;
var minX = 400;
var checkedRGB = false;
var _checkedStd = false;
var _endmemberColors = [];

// sizes =======================================================================
var _lineChartWidth;
var _lineChartHeight;

// UI ==========================================================================
var _endmemberLines, _stdAreas;

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
    .domain([minX, maxX])
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
    .attr("stroke", "none")
    .attr("opacity", 0);

// Add the line for the selection
var _selectionLine = _lineChart.append("path")
    .attr("class", "selectionLine")
    .attr("opacity", 1)
    .attr("fill", "none")
    .attr("stroke", "black")
    .attr("stroke-width", 2);

// Create a rect on top of the svg area: this rectangle recovers mouse position
var _topRect = _lineChart
    .append('rect')
    .style("fill", "none")
    .style("pointer-events", "all")
    .attr('width', _lineChartWidth)
    .attr('height', _lineChartHeight)
    .on('mouseover', mouseover)
    .on('mousemove', mousemove)
    .on('mousedown', mousedown)
    .on('mouseup', mouseup);


drawRGBlines();

// Create the text that travels along the curve of chart
var focusText = _lineChart
    .append('g')
    .append('text')
    .style("opacity", 1)
    .attr("text-anchor", "left")
    .attr("alignment-baseline", "middle");

var line = d3.line()
    .x(function (d) { return x(d.x); })
    .y(function (d) { return y(d.y); });

var area = d3.area()
    .x(function (d) { return x(d.x) })
    .y0(function (d) { return y(d.CI_Right) })
    .y1(function (d) { return y(d.CI_Left) });

function drawLineChart(dur) {

    // append the svg object to the body of the page
    _svgSelection
        .attr("width", _lineChartWidth + _margin.left + _margin.right)
        .attr("height", _lineChartHeight + _margin.top + _margin.bottom)
    
    x.range([0, _lineChartWidth]);
    _lineChart.select("g.xAxis")
        .transition().duration(dur)
        .attr("transform", "translate(0," + _lineChartHeight + ")")
        .call(d3.axisBottom(x));

    xLabel
        .transition().duration(dur)
        .attr("x", _lineChartWidth)
        .attr("y", _lineChartHeight - 6);

    y.range([_lineChartHeight, 0]);
    _lineChart.select("g.yAxis")
        .transition().duration(dur)
        .call(d3.axisLeft(y));

    _topRect
        .attr('width', _lineChartWidth)
        .attr('height', _lineChartHeight);

    updateRGBlines();

    if (_data) {

        _stdArea
            .datum(_data)
            .transition().duration(dur)
            .attr("d", area);

        _selectionLine
            .datum(_data)
            .transition().duration(dur)
            .attr("d", line);
    }

    if (_endmembers.length > 0) {

        _stdAreas
            .data(_endmembers)
            .transition().duration(dur)
            .attr("d", area);

        _endmemberLines
            .data(_endmembers)
            .transition().duration(dur)
            .attr("d", line);
            
    }
}

function mouseover() {

    if (!_data) return;

    _topRect.raise();

    // recover coordinate we need
    var x0 = x.invert(d3.mouse(this)[0]);
    var i = bisect(_data, x0, 1);

    if (_data[i]) {

        selectedData = _data[i];

        var distR = Math.abs(selectedData.x - wavelengthR);
        var distG = Math.abs(selectedData.x - wavelengthG);
        var distB = Math.abs(selectedData.x - wavelengthB);
        var maxDist = 10;


        if (distR < maxDist) {
            lineR.attr("stroke-width", 6);
        }
        else if (distG < maxDist) {
            lineG.attr("stroke-width", 6);
        }
        else if (distB < maxDist) {
            lineB.attr("stroke-width", 6);
        }
    }
}

function mousedown() {

    if (!_data) return;

    _topRect.raise();

    // recover coordinate we need
    var x0 = x.invert(d3.mouse(this)[0]);
    var i = bisect(_data, x0, 1);

    if (_data[i]) {

        selectedData = _data[i];

        var distR = Math.abs(selectedData.x - wavelengthR);
        var distG = Math.abs(selectedData.x - wavelengthG);
        var distB = Math.abs(selectedData.x - wavelengthB);
        var maxDist = 10;


        if (distR < maxDist) {
            moveLine = "R";
            lineR.attr("stroke-width", 2);
        }
        else if (distG < maxDist) {
            moveLine = "G";
            lineG.attr("stroke-width", 2);
        }
        else if (distB < maxDist) {
            moveLine = "B";
            lineB.attr("stroke-width", 2);
        }
    }
}

function mousemove() {

    if (!_data) return;

    _topRect.raise();

    lineR.attr("stroke-width", 2);
    lineG.attr("stroke-width", 2);
    lineB.attr("stroke-width", 2);

    // recover coordinate we need
    var x0 = x.invert(d3.mouse(this)[0]);
    var i = bisect(_data, x0, 1);

    if (_data[i]) {
        selectedData = _data[i];

        if (moveLine != "N") {
            var xValue = Number(selectedData.x).toFixed(0);
            var yValue = Number(selectedData.y).toFixed(3);

            focusText
                .html("x:" + xValue + ", " + "y:" + yValue)
                .attr("x", x(selectedData.x) + _lineChartWidth / 90)
                .attr("y", 3 * _lineChartHeight / 5)
                .style("opacity", 1);

            if (moveLine == "R") {
                lineR
                    .attr("x1", x(selectedData.x))
                    .attr("y1", y(minY))
                    .attr("x2", x(selectedData.x))
                    .attr("y2", y(maxY));
            }
            else if (moveLine == "G") {
                lineG
                    .attr("x1", x(selectedData.x))
                    .attr("y1", y(minY))
                    .attr("x2", x(selectedData.x))
                    .attr("y2", y(maxY));
            }
            else if (moveLine == "B") {
                lineB
                    .attr("x1", x(selectedData.x))
                    .attr("y1", y(minY))
                    .attr("x2", x(selectedData.x))
                    .attr("y2", y(maxY));
            }
        }
        else {
            var distR = Math.abs(selectedData.x - wavelengthR);
            var distG = Math.abs(selectedData.x - wavelengthG);
            var distB = Math.abs(selectedData.x - wavelengthB);
            var maxDist = 10;

            if (distR < maxDist) {
                lineR.attr("stroke-width", 6);
            }
            else if (distG < maxDist) {
                lineG.attr("stroke-width", 6);
            }
            else if (distB < maxDist) {
                lineB.attr("stroke-width", 6);
            }
        }
    }
}

function mouseup() {

    if (!_data) return;

    _topRect.raise();

    // recover coordinate we need
    var x0 = x.invert(d3.mouse(this)[0]);
    var i = bisect(_data, x0, 1);

    if (_data[i]) {
        selectedData = _data[i];

        if (moveLine == "R") {
            wavelengthR = selectedData.x;

            lineR
                .attr("x1", x(wavelengthR))
                .attr("y1", y(minY))
                .attr("x2", x(wavelengthR))
                .attr("y2", y(maxY));

            // send RGB wavelength values to qt
            sendRGBWavelengths();
        }
        else if (moveLine == "G") {
            wavelengthG = selectedData.x;
            lineG
                .attr("x1", x(wavelengthG))
                .attr("y1", y(minY))
                .attr("x2", x(wavelengthG))
                .attr("y2", y(maxY));

            // send RGB wavelength values to qt
            sendRGBWavelengths();
        }
        else if (moveLine == "B") {
            wavelengthB = selectedData.x;
            lineB
                .attr("x1", x(wavelengthB))
                .attr("y1", y(minY))
                .attr("x2", x(wavelengthB))
                .attr("y2", y(maxY));

            // send RGB wavelength values to qt
            sendRGBWavelengths();
        }

        moveLine = "N";
        focusText.style("opacity", 0);
    }
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
    _data = JSON.parse(d);
    addData();
}

function setEndmemberColor(r, g, b, row) {
    var color = [r, g, b];

    _endmemberColors[row] = color;

    if (_endmembers.length > 0) {
        _lineChart.select("#area" + row).attr("fill", d3.rgb(r, g, b));
        _lineChart.select("#line" + row).attr("stroke", d3.rgb(r, g, b));
    }
}

function setEndmemberVisibility(toggled, row) {

    if (toggled) {
        showElement("#line" + row, 1);

        if (_checkedStd) {
            showElement("#area" + row, 0.1);
        }
    }
    else {
        removeElement("#area" + row);
        removeElement("#line" + row);
    }
}

function setEndmember(d) {
    endmember = JSON.parse(d);

    _endmembers.push(endmember);

    addEndmembers(endmember, (_endmembers.length - 1));
}

function setEndmemberRemoved(row) {

    _lineChart.select("#area" + row).remove();
    _lineChart.select("#line" + row).remove();

    _endmemberColors.splice(row, 1);

    var noEndmembers = _endmembers.length;
    log("No endmembers: " + noEndmembers);
    log("Row removed: " + row);

    if (noEndmembers > 1) {

        // Update all remaining indices
        for (var i = row + 1; i < noEndmembers; i++) {
            var newIndex = i - 1;

            _lineChart.select("#area" + i).attr("id", "area" + newIndex);
            _lineChart.select("#line" + i).attr("id", "line" + newIndex);
        }
    }

    _endmembers.splice(row, 1);

}

function setHighlightSelection(row) {

    log(row);
    _lineChart.selectAll(".endmembers").attr("stroke-width", 2);
    _lineChart.select("#line" + row).attr("stroke-width", 4);
}

function enableRGBLines(c) {
    checkedRGB = c;

    if (checkedRGB) {
        showElement(".RGBLines", 1);
    } else {
        removeElement(".RGBLines");
    }
}

function enableStdArea(c) {
    _checkedStd = c;

    if (_checkedStd) {
        showElement(".stdInterval", 0.1);
        showElement(".endmembersStd", 0.1);
    }
    else {
        removeElement(".stdInterval");
        removeElement(".endmembersStd");
    }
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
    
    drawLineChart(1000);
}

resize();
d3.select(window).on("resize", resize);
