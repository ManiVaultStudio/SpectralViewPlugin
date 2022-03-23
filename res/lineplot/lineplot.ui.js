var moveLine = "N";
var spectrumNo = 0;
var colors = ["gold", "blue", "green", "black", "grey", "darkblue", "darkgreen", "pink", "brown", "purple", "grey1", "orange"];

// This allows to find the closest X index of the mouse:
var bisect = d3.bisector(function (d) { return d.x; }).left;

function addEndmembers() {
    
    _stdAreas.exit();

    _stdAreas.enter()
        .append("path")
        .attr("class", "endmembersStd")
        .attr("fill", "red")
        .attr("stroke", "none")
        .attr("opacity", 0)
        .attr("d", area);

    _endmemberLines.exit();

    _endmemberLines.enter()
        .append("path")
        .attr("class", "endmembers")
        .attr("opacity", 1)
        .attr("fill", "none")
        .attr("stroke", "red")
        .attr("stroke-width", 2)
        .attr("d", line);
}

function addData() {

    // Change axes domain
    if (spectrumNo == 0) {
        x.domain(d3.extent(_data, function (d) { return d.x; }));

        _lineChart.selectAll("g.xAxis")
            .transition().duration(1000)
            .call(d3.axisBottom(x));

        updateRGBlines();
    }

    var max1 = d3.max(_data, function (d) { return +d.y; });
    var min1 = d3.min(_data, function (d) { return +d.y; });

    var newYMax = d3.max(_data, function (d) { return +d.CI_Right; });
    var newYMin = d3.min(_data, function (d) { return +d.CI_Left; });

    if (max1 > newYMax) {
        newYMax = max1;
        newYMin = min1;
    }

    if (newYMax > maxY || newYMin < minY) {

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

    spectrumNo++;

    if (spectrumNo > colors.length) {
        spectrumNo = 0;
    }
}

function mouseover() {

    if (!_data) return;

    // recover coordinate we need
    var x0 = x.invert(d3.mouse(this)[0]);
    var i = bisect(_data, x0, 1);
    selectedData = _data[i];
    log(selectedData.x);
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
