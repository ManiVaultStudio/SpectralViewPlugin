var lineR, lineG, lineB;
var setRGBCheck, setRGBLabel, setRGB;
var colorsRGB;
var wavelengthR = 630;
var wavelengthG = 532;
var wavelengthB = 465;

function drawRGBlines() {
    colorsRGB = ["red", "green", "blue"];

    // Create the lines corresponding to RGB wavelengths
    lineR = _lineChart
        .append('g')
        .append('line')
        .attr("stroke", colorsRGB[0])
        .style("opacity", 0)
        .attr("class", "RGBLines")
        .attr("stroke-width", 2)
        .attr("x1", x(wavelengthR))
        .attr("y1", y(minY))
        .attr("x2", x(wavelengthR))
        .attr("y2", y(maxY));

    lineG = _lineChart
        .append('g')
        .append('line')
        .attr("stroke", colorsRGB[1])
        .style("opacity", 0)
        .attr("class", "RGBLines")
        .attr("stroke-width", 2)
        .attr("x1", x(wavelengthG))
        .attr("y1", y(minY))
        .attr("x2", x(wavelengthG))
        .attr("y2", y(maxY));

    lineB = _lineChart
        .append('g')
        .append('line')
        .attr("stroke", colorsRGB[2])
        .style("opacity", 0)
        .attr("class", "RGBLines")
        .attr("stroke-width", 2)
        .attr("x1", x(wavelengthB))
        .attr("y1", y(minY))
        .attr("x2", x(wavelengthB))
        .attr("y2", y(maxY));
}

function updateRGBlines() {

    lineR
        .transition().duration(1000)
        .attr("x1", x(wavelengthR))
        .attr("y1", y(minY))
        .attr("x2", x(wavelengthR))
        .attr("y2", y(maxY));

    lineG
        .transition().duration(1000)
        .attr("x1", x(wavelengthG))
        .attr("y1", y(minY))
        .attr("x2", x(wavelengthG))
        .attr("y2", y(maxY));

    lineB
        .transition().duration(1000)
        .attr("x1", x(wavelengthB))
        .attr("y1", y(minY))
        .attr("x2", x(wavelengthB))
        .attr("y2", y(maxY));
}