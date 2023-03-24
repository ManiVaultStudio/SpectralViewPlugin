var moveLine = "N";

// This allows to find the closest X index of the mouse:
var bisect = d3.bisector(function (d) { return d.x; }).left;

function addData() {

    // y-axis
    // selection mean
    //var e_min = d3.min(_data, function (d) { return +d.y; });
    var e_max = d3.max(_data, function (d) { return +d.y; });

    // selection confidence intervals
    //e_min = Math.min(e_min, d3.min(_data, function (d) { return +d.CI_Left; }));
    if (_checkedStd) { e_max = Math.max(e_max, d3.max(_data, function (d) { return +d.CI_Right; })); }

    //log(_endmembers);
    // endmembers
    for (let i = 0; i < _endmembers.length; i++) {
        //e_min = Math.min(e_min, d3.min(e, function (d) { return +d.y; }));
        //e_min = Math.min(e_min, d3.min(e, function (d) { return +d.CI_Left; }));

        //log(_endmembers[i]);

        e_max = Math.max(e_max, d3.max(_endmembers[i], function (d) { return +d.y; }));
        if(_checkedStd) { e_max = Math.max(e_max, d3.max(_endmembers[i], function (d) { return +d.CI_Right; })); }
    }

    if (e_max != maxY) {// || e_min != minY) {
        updateYAxis(e_max, 0);
    }

    // x-axis
    var x_min = d3.min(_data, function (d) { return +d.x; });
    var x_max = d3.max(_data, function (d) { return +d.x; });

    for (const e in _endmembers) {
        x_min = Math.min(x_min, d3.min(_data, function (d) { return +d.x; }));
        x_max = Math.max(x_max, d3.max(_data, function (d) { return +d.x; }));
    }

    if (x_max != maxX || x_min != minX) {
        updateXAxis(x_max, x_min);
    }

    // Set confidence interval
    _stdArea
        .datum(_data)
        .transition().duration(200)
        .attr("d", area);

    // Add the line
    _selectionLine
        .datum(_data)
        .transition().duration(200)
        .attr("d", line);

    if (_endmembers.length > 0) {

        _stdAreas
            .data(_endmembers)
            .transition().duration(200)
            .attr("d", area);

        _endmemberLines
            .data(_endmembers)
            .transition().duration(200)
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

    maxX = newXMax;
    minX = newXMin;

    x.domain([minX, maxX]);

    _lineChart.selectAll("g.xAxis")
        .transition().duration(200)
        .call(d3.axisBottom(x));

    updateRGBlines();
}

function updateYAxis(newYMax, newYMin) {

    maxY = newYMax;
    minY = newYMin;

    y.domain([minY, maxY]);

    _lineChart.selectAll("g.yAxis")
        .transition().duration(200)
        .call(d3.axisLeft(y));

    updateRGBlines();
}