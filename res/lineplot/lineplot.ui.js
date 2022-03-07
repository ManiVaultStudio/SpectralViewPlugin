// set the dimensions and margins of the lineplot
var margin = { top: 10, right: 80, bottom: 30, left: 60 },
    width = 800 - margin.left - margin.right,
    height = 700 - margin.top - margin.bottom;

var x, y;
var lineR, lineG, lineB;
var setRGBCheck, setRGBLabel, setRGB;
var moveLine = "N";
var moving = 0;
var distR, distG, distB, maxDist;

var colors;
var wavelengthR = 630;
var wavelengthG = 532;
var wavelengthB = 465;

// append the svg object to the body of the page
var svg = d3.select("#line_chart")
    .append("svg")
    .attr("width", width + margin.left + margin.right)
    .attr("height", height + margin.top + margin.bottom)
    .append("g")
    .attr("transform",
        "translate(" + margin.left + "," + margin.top + ")");

// delte
//var data = "[{\"x\":10, \"y\":0.5}, {\"x\":500, \"y\":0.4}, {\"x\":800, \"y\":0.9}]";
//var _data = JSON.parse(data);
//drawLineChart();

function drawLineChart() {

    // Add X axis --> it is a date format
    x = d3.scaleLinear()
        .domain(d3.extent(_data, function (d) { return d.x; }))
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
        .domain([0, d3.max(_data, function (d) { return +d.y; })])
        .range([height, 0]);
    svg.append("g")
        .call(d3.axisLeft(y));

    // Y axis label
    svg.append("text")
        .attr("class", "yLabel")
        .attr("text-anchor", "end")
        .attr("y", 6)
        .attr("dy", ".75em")
        .attr("transform", "rotate(-90)")
        .text("Value");

    // Add the line
    svg.append("path")
        .datum(_data)
        .attr("fill", "none")
        .attr("stroke", "black")
        .attr("stroke-width", 2)
        .attr("d", d3.line()
            .x(function (d) { return x(d.x) })
            .y(function (d) { return y(d.y) })
    )

    // Options menu
    drawRGBlines();
    setRGBCheckbox();

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
        //.on('mouseout', mouseout)
        .on("click", mouseclick);

    // What happens when the mouse move -> show the annotations at the right positions.
    function mouseover() {

        /*
        if (moving == 0) {

            // recover coordinate we need
            var x0 = x.invert(d3.mouse(this)[0]);
            var i = bisect(_data, x0, 1);
            selectedData = _data[i];

            distR = Math.abs(selectedData.x - wavelengthR);
            distG = Math.abs(selectedData.x - wavelengthG);
            distB = Math.abs(selectedData.x - wavelengthB);
            maxDist = 10;

            if (distR < maxDist) {
                log("jello");
                var lineRFocus = svg
                    .append('g')
                    .append('line')
                    .style("stroke", colors[0])
                    .style("opacity", 0.5)
                    .attr("stroke-width", 4)
                    .attr("x1", x(wavelengthG))
                    .attr("y1", y(0))
                    .attr("x2", x(wavelengthG))
                    .attr("y2", y(height - 10));
            }

        }
        */
        
    }

    function mousemove() {

        //var _data = JSON.parse(data);

        if (moving == 1) {
            // recover coordinate we need
            var x0 = x.invert(d3.mouse(this)[0]);
            var i = bisect(_data, x0, 1);
            selectedData = _data[i];
            var xValue = Number(selectedData.x).toFixed(0);
            var yValue = Number(selectedData.y).toFixed(3);

            if (moveLine != "N") {
                focusText
                    .html("x:" + xValue + "\n" + "y:" + yValue)
                    .attr("x", x(selectedData.x) + 15)
                    .attr("y", y(selectedData.y))
            }

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
    }

  //  function mouseout() {
  //      focus.style("opacity", 0)
  //      focusText.style("opacity", 0)
   // }

    function mouseclick() {

       // var _data = JSON.parse(data);

        if (moving == 0) {

            focusText.style("opacity", 1);

            // recover coordinate we need
            var x0 = x.invert(d3.mouse(this)[0]);
            var i = bisect(_data, x0, 1);
            selectedData = _data[i];

            distR = Math.abs(selectedData.x - wavelengthR);
            distG = Math.abs(selectedData.x - wavelengthG);
            distB = Math.abs(selectedData.x - wavelengthB);
            maxDist = 10;

            log(selectedData.x);
            if (distR < maxDist) {
                moveLine = "R";
            }
            else if (distG < maxDist) {
                moveLine = "G";
            }
            else if (distB < maxDist) {
                moveLine = "B";
            }

            moving = 1;
        }
        else if (moving = 1) {

            focusText.style("opacity", 0);

            if (moveLine == "R") {
                wavelengthR = selectedData.x;
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

            moving = 0;
        }
    }
}

function drawRGBlines() {
    colors = ["red", "green", "blue"];

    // Create the lines corresponding to RGB wavelengths
    lineR = svg
        .append('g')
        .append('line')
        .style("stroke", colors[0])
        .style("opacity", 0)
        .attr("stroke-width", 2)
        .attr("x1", x(wavelengthR))
        .attr("y1", y(0))
        .attr("x2", x(wavelengthR))
        .attr("y2", y(height - 10));

    lineG = svg
        .append('g')
        .append('line')
        .style("stroke", colors[1])
        .style("opacity", 0)
        .attr("stroke-width", 2)
        .attr("x1", x(wavelengthG))
        .attr("y1", y(0))
        .attr("x2", x(wavelengthG))
        .attr("y2", y(height - 10));

    lineB = svg
        .append('g')
        .append('line')
        .style("stroke", colors[2])
        .style("opacity", 0)
        .attr("stroke-width", 2)
        .attr("x1", x(wavelengthB))
        .attr("y1", y(0))
        .attr("x2", x(wavelengthB))
        .attr("y2", y(height - 10));
}

function showRGBlines() {

    lineR.style("opacity", 1);
    lineG.style("opacity", 1);
    lineB.style("opacity", 1);
}

function removeRGBlines() {

    lineR.style("opacity", 0);
    lineG.style("opacity", 0);
    lineB.style("opacity", 0);
}

function setRGBCheckbox() {

    setRGBCheck = document.createElement('input');
    setRGBCheck.type = 'checkbox';
    setRGBCheck.id = 'setRGBCheck';

    setRGBLabel = document.createElement('label');
    setRGBLabel.append("Set RGB wavelength values");

    setRGB = document.createElement('div');
    setRGB.id = "setRGB";
    setRGB.appendChild(setRGBCheck);
    setRGB.appendChild(setRGBLabel);
    document.getElementById("options").appendChild(setRGB);

    setRGBCheck.addEventListener('change', (event) => {
        if (event.currentTarget.checked) {
            showRGBlines();
        } else {
            removeRGBlines();
        }
    })
}
