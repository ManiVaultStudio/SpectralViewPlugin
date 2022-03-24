function addEndmembers(endmember, index) {

    var newYMax = d3.max(endmember, function (d) { return +d.y; });

    if (newYMax > maxY) {

        maxY = newYMax;

        y.domain([minY, maxY]);

        _lineChart.selectAll("g.yAxis")
            .transition().duration(1000)
            .call(d3.axisLeft(y));

        updateRGBlines();
    }

    var r = _endmemberColors[index][0];
    var g = _endmemberColors[index][1];
    var b = _endmemberColors[index][2];

    var newEndmemberStd = _lineChart.append("path")
        .datum(endmember)
        .attr("class", "endmembersStd")
        .attr("fill", d3.rgb(r, g, b))
        .attr("stroke", "none")
        .attr("d", area);

    var newEndmember = _lineChart.append("path")
        .datum(endmember)
        .attr("class", "endmembers")
        .attr("opacity", 1)
        .attr("fill", "none")
        .attr("stroke", d3.rgb(r, g, b))
        .attr("stroke-width", 2)
        .attr("d", line);

    if (_checkedStd) {
        newEndmemberStd.attr("opacity", 0.1);
    }
    else {
        newEndmemberStd.attr("opacity", 0);
    }

    // Save the lines for the endmembers
    _endmemberLines = _lineChart.selectAll(".endmembers").data(_endmembers);

    // SAve the confidence intervals for the endmembers
    _stdAreas = _lineChart.selectAll(".endmembersStd").data(_endmembers);
}
