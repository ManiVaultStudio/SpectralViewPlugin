var moveLine = "N";

// This allows to find the closest X index of the mouse:
var bisect = d3.bisector(function (d) { return d.x; }).left;

function addData() {

    var max1 = d3.max(_data, function (d) { return +d.y; });
    var min1 = d3.min(_data, function (d) { return +d.y; });

    var newYMax = d3.max(_data, function (d) { return +d.CI_Right; });
    var newYMin = d3.min(_data, function (d) { return +d.CI_Left; });

    if (max1 > newYMax) {
        newYMax = max1;
        newYMin = min1;
    }

    if (newYMax > maxY || newYMin < minY) {
        updateYAxis(newYMax, newYMin);
    }

    var newXMax = d3.max(_data, function (d) { return +d.x; });
    var newXMin = d3.min(_data, function (d) { return +d.x; });

    if (newXMax > maxX || newXMin < minX) {
        updateXAxis(newXMax, newXMin);
    }

    // Set confidence interval
    _stdArea
        .datum(_data)
        .transition().duration(1000)
        .attr("d", area);

    // Add the line
    _selectionLine
        .datum(_data)
        .transition().duration(1000)
        .attr("d", line)
}

function mouseover() {

    if (!_data) return;

    // recover coordinate we need
    var x0 = x.invert(d3.mouse(this)[0]);
    var i = bisect(_data, x0, 1);
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

function mousedown() {

    if (!_data) return;

    // recover coordinate we need
    var x0 = x.invert(d3.mouse(this)[0]);
    var i = bisect(_data, x0, 1);
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

function mousemove() {

    if (!_data) return;

    lineR.attr("stroke-width", 2);
    lineG.attr("stroke-width", 2);
    lineB.attr("stroke-width", 2);

    // recover coordinate we need
    var x0 = x.invert(d3.mouse(this)[0]);
    var i = bisect(_data, x0, 1);
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

function mouseup() {

    if (!_data) return;

    // recover coordinate we need
    var x0 = x.invert(d3.mouse(this)[0]);
    var i = bisect(_data, x0, 1);
    selectedData = _data[i];

    if (moveLine == "R") {
        wavelengthR = selectedData.x;

        lineR
            .attr("x1", x(wavelengthR))
            .attr("y1", y(minY))
            .attr("x2", x(wavelengthR))
            .attr("y2", y(maxY));
    }
    else if (moveLine == "G") {
        wavelengthG = selectedData.x;
        lineG
            .attr("x1", x(wavelengthG))
            .attr("y1", y(minY))
            .attr("x2", x(wavelengthG))
            .attr("y2", y(maxY));
    }
    else if (moveLine == "B") {
        wavelengthB = selectedData.x;
        lineB
            .attr("x1", x(wavelengthB))
            .attr("y1", y(minY))
            .attr("x2", x(wavelengthB))
            .attr("y2", y(maxY));
    }

    moveLine = "N";
    focusText.style("opacity", 0);

    // send RGB wavelength values to qt
    sendRGBWavelengths();
}

function showElement(name, value) {

    _lineChart.selectAll(name).style("opacity", value);
}

function removeElement(name) {

    _lineChart.selectAll(name).style("opacity", 0);
}

function updateXAxis(newXMax, newXMin) {

    if (newXMax > maxX) {
        maxX = newXMax;
    }

    if (newXMin < minX) {
        minX = newXMin;
    }

    x.domain([minX, maxX]);

    _lineChart.selectAll("g.xAxis")
        .transition().duration(1000)
        .call(d3.axisBottom(x));

    updateRGBlines();
}

function updateYAxis(newYMax, newYMin) {

    if (newYMax > maxY) {
        maxY = newYMax;
    }
    if (newYMin < minY) {
        minY = newYMin;
    }

    y.domain([minY, maxY]);

    _lineChart.selectAll("g.yAxis")
        .transition().duration(1000)
        .call(d3.axisLeft(y));

    updateRGBlines();
}