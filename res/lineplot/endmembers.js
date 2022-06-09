function addEndmembers(endmember, index) {

    _visibleEndmembers[index] = false;

    var newYMax = d3.max(endmember, function (d) {
        if (d.CI_Right != 0)
            return +d.CI_Right;
        else
            return +d.y;
    });
    var newYMin = d3.min(endmember, function (d) { return +d.CI_Left; });

    if (newYMax > maxY || newYMin < minY) {

        updateYAxis(newYMax, newYMin);
    }

    var newXMax = d3.max(endmember, function (d) { return +d.x; });
    var newXMin = d3.min(endmember, function (d) { return +d.x; });

    if (newXMax > maxX || newXMin < minX) {
        updateXAxis(newXMax, newXMin);
    }

    var r = _endmemberColors[index][0];
    var g = _endmemberColors[index][1];
    var b = _endmemberColors[index][2];

    var newEndmemberStd = _lineChart.append("path")
        .datum(endmember)
        .attr("class", "endmembersStd")
        .attr("fill", d3.rgb(r, g, b))
        .attr("stroke", "none")
        .attr("id", "area" + index)
        .style("opacity", 0)
        .attr("d", area);

    var newEndmember = _lineChart.append("path")
        .datum(endmember)
        .attr("class", "endmembers")
        .style("opacity", 0)
        .attr("fill", "none")
        .attr("stroke", d3.rgb(r, g, b))
        .attr("stroke-width", 2)
        .attr("id", "line" + index)
        .attr("d", line);

    // Save the lines for the endmembers
    _endmemberLines = _lineChart.selectAll(".endmembers").data(_endmembers);

    // Save the confidence intervals for the endmembers
    _stdAreas = _lineChart.selectAll(".endmembersStd").data(_endmembers);

    if (_data) {

        _stdArea
            .datum(_data)
            .transition().duration(1000)
            .attr("d", area);

        _selectionLine
            .datum(_data)
            .transition().duration(1000)
            .attr("d", line);
    }

    _stdAreas
        .data(_endmembers)
        .transition().duration(1000)
        .attr("d", area);

    _endmemberLines
        .data(_endmembers)
        .transition().duration(1000)
        .attr("d", line);
}
