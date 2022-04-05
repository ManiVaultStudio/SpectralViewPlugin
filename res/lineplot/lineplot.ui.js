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
        .attr("d", line);

    if (_endmembers.length > 0) {

        _stdAreas
            .data(_endmembers)
            .transition().duration(1000)
            .attr("d", area);

        _endmemberLines
            .data(_endmembers)
            .transition().duration(1000)
            .attr("d", line);

    }
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