var lineR, lineG, lineB;
var setRGBCheck, setRGBLabel, setRGB;
var colorsRGB;
var wavelengthR = 630;
var wavelengthG = 532;
var wavelengthB = 465;

function drawRGBlines() {
    colorsRGB = ["red", "green", "blue"];

    // Create the lines corresponding to RGB wavelengths
    lineR = svg
        .append('g')
        .append('line')
        .style("stroke", colorsRGB[0])
        .style("opacity", 0)
        .attr("stroke-width", 2)
        .attr("x1", x(wavelengthR))
        .attr("y1", y(0))
        .attr("x2", x(wavelengthR))
        .attr("y2", y(height - 10));

    lineG = svg
        .append('g')
        .append('line')
        .style("stroke", colorsRGB[1])
        .style("opacity", 0)
        .attr("stroke-width", 2)
        .attr("x1", x(wavelengthG))
        .attr("y1", y(0))
        .attr("x2", x(wavelengthG))
        .attr("y2", y(height - 10));

    lineB = svg
        .append('g')
        .append('line')
        .style("stroke", colorsRGB[2])
        .style("opacity", 0)
        .attr("stroke-width", 2)
        .attr("x1", x(wavelengthB))
        .attr("y1", y(0))
        .attr("x2", x(wavelengthB))
        .attr("y2", y(height - 10));
}


function updateRGBlines() {

    lineR
        .attr("x1", x(wavelengthR))
        .attr("y1", y(0))
        .attr("x2", x(wavelengthR))
        .attr("y2", y(height - 10));

    lineG
        .attr("x1", x(wavelengthG))
        .attr("y1", y(0))
        .attr("x2", x(wavelengthG))
        .attr("y2", y(height - 10));

    lineB
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