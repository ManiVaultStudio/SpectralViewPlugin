// =============================================================================
// find out if the QtBridge is available
// otherwise we are running in the browser
// =============================================================================
try {
    new QWebChannel(qt.webChannelTransport, function (channel) {

        QtBridge = channel.objects.QtBridge;

        QtBridge.qt_setData.connect(function () { setData(arguments[0]); });
        //QtBridge.qt_setSelection.connect(function () { setSelection(arguments[0]); });
        //QtBridge.qt_setHighlight.connect(function () { setHighlight(arguments[0]); });
        QtBridge.qt_addAvailableData.connect(function () { addAvailableData(arguments[0]); });
        //QtBridge.qt_setMarkerSelection.connect(function () { initMarkerSelection(arguments[0]); });

        notifyBridgeAvailable();
    });
} catch (error) {
    isQtAvailable = false;
    log("could not connect qt");
}

// =============================================================================
// globals
// =============================================================================

// data ========================================================================
var _data = null;
var _spectra = [];
var _availableDataSets = [];

// =============================================================================
// external
// =============================================================================
function addAvailableData(name) {

    for (var i = 0; i < _availableDataSets.length; i++) {
        if (name == _availableDataSets[i]) {
            return;
        }
    }

    _availableDataSets.push(name);

    //updateAvailableDataSelectionBox();
}

function setData(d) {

    log("setting data");

    _data = JSON.parse(d);
    addData();

    log("Data set.");
}

function sendRGBWavelengths() {

    if (isQtAvailable) {
        QtBridge.js_setRGBWavelengths(wavelengthR, wavelengthG, wavelengthB);
    }
}