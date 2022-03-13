// set the dimensions and margins of the lineplot
var margin = { top: 10, right: 80, bottom: 30, left: 60 },
    width = 800 - margin.left - margin.right,
    height = 700 - margin.top - margin.bottom;

var x, y, line;
var moveLine = "N";
var spectrumNo = 0;
var maxY = 0.1;
var colors = ["gold", "blue", "green", "black", "grey", "darkblue", "darkgreen", "pink", "brown", "purple", "grey1", "orange"];

// append the svg object to the body of the page
var svg = d3.select("#line_chart")
    .append("svg")
    .attr("width", width + margin.left + margin.right)
    .attr("height", height + margin.top + margin.bottom)
    .append("g")
    .attr("transform",
        "translate(" + margin.left + "," + margin.top + ")");

drawLineChart();

// Options menu
drawRGBlines();
setRGBCheckbox();

function drawLineChart() {

    // Add X axis --> it is a date format
    x = d3.scaleLinear()
        .domain([400, 900])
        .range([0, width]);
    svg.append("g")
        .attr("class", "xAxis")
        .attr("transform", "translate(0," + height + ")")
        .call(d3.axisBottom(x));

    // X axis label
    svg.append("text")
        .attr("class", "xLabel")
        .attr("text-anchor", "end")
        .attr("x", width)
        .attr("y", height - 6)
        .text("Wavelength (nm)");

    // Add Y axis
    y = d3.scaleLinear()
        .domain([0, maxY])
        .range([height, 0]);
    svg.append("g")
        .attr("class", "yAxis")
        .call(d3.axisLeft(y));

    // Y axis label
    svg.append("text")
        .attr("class", "yLabel")
        .attr("text-anchor", "end")
        .attr("y", 6)
        .attr("dy", ".75em")
        .attr("transform", "rotate(-90)")
        .text("Value");

    line = d3.line()
        .x(function (d) { return x(d.x); })
        .y(function (d) { return y(d.y); });
}


function addData() {
    
    // Change axes domain
    if (spectrumNo == 0) {
        x.domain(d3.extent(_data, function (d) { return d.x; }));

        svg.selectAll("g.xAxis")
            .transition().duration(1000)
            .call(d3.axisBottom(x));

        updateRGBlines();
    }

    var newYMax = d3.max(_data, function (d) { return +d.y; });

    if (newYMax > maxY) {
        maxY = newYMax;

        y.domain([0, maxY]);

        svg.selectAll("g.yAxis")
            .transition().duration(2000)
            .call(d3.axisLeft(y));

        if (_spectra.length != 0) {

            var paths = svg.selectAll(".lines").data(_spectra)
                .transition().duration(2000)
                .attr("d", line);
        }
    }

    // Add the line
    svg.append("path")
        .datum(_data)
        .attr("class", "lines")
        .attr("fill", "none")
        .attr("stroke", colors[spectrumNo])
        .attr("stroke-width", 2)
        .attr("d", line)

    _spectra.push(_data);
    spectrumNo++;

    if (spectrumNo > colors.length) {
        spectrumNo = 0;
    }

    // This allows to find the closest X index of the mouse:
    var bisect = d3.bisector(function (d) { return d.x; }).left;

    // Create the text that travels along the curve of chart
    var focusText = svg
        .append('g')
        .append('text')
        .style("opacity", 1)
        .attr("text-anchor", "left")
        .attr("alignment-baseline", "middle");

    // Create a rect on top of the svg area: this rectangle recovers mouse position
    svg
        .append('rect')
        .style("fill", "none")
        .style("pointer-events", "all")
        .attr('width', width)
        .attr('height', height)
        .on('mouseover', mouseover)
        .on('mousemove', mousemove)
        .on('mousedown', mousedown)
        .on('mouseup', mouseup);

    function mouseover() {
        
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
                .attr("x", x(selectedData.x) + width / 90)
                .attr("y", 3 * height / 5)
                .style("opacity", 1);

            if (moveLine == "R") {
                lineR
                    .attr("x1", x(selectedData.x))
                    .attr("y1", y(0))
                    .attr("x2", x(selectedData.x))
                    .attr("y2", y(height - 10));
            }
            else if (moveLine == "G") {
                lineG
                    .attr("x1", x(selectedData.x))
                    .attr("y1", y(0))
                    .attr("x2", x(selectedData.x))
                    .attr("y2", y(height - 10));
            }
            else if (moveLine == "B") {
                lineB
                    .attr("x1", x(selectedData.x))
                    .attr("y1", y(0))
                    .attr("x2", x(selectedData.x))
                    .attr("y2", y(height - 10));
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

        // recover coordinate we need
        var x0 = x.invert(d3.mouse(this)[0]);
        var i = bisect(_data, x0, 1);
        selectedData = _data[i];
        if (moveLine == "R") {
            wavelengthR = selectedData.x;
            log("R: " + wavelengthR);
            lineR
                .attr("x1", x(wavelengthR))
                .attr("y1", y(0))
                .attr("x2", x(wavelengthR))
                .attr("y2", y(height - 10));
        }
        else if (moveLine == "G") {
            wavelengthG = selectedData.x;
            lineG
                .attr("x1", x(wavelengthG))
                .attr("y1", y(0))
                .attr("x2", x(wavelengthG))
                .attr("y2", y(height - 10));
        }
        else if (moveLine == "B") {
            wavelengthB = selectedData.x;
            lineB
                .attr("x1", x(wavelengthB))
                .attr("y1", y(0))
                .attr("x2", x(wavelengthB))
                .attr("y2", y(height - 10));
        }

        moveLine = "N";
        focusText.style("opacity", 0);

        // send RGB wavelength values to qt
        sendRGBWavelengths();
    }
}

