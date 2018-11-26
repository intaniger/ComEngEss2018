// {"id":2861,"posX":427.6,"posY":355.2,"angleX":90,"angleY":0,"angleZ":0}
let state = 0;
let canvas = {
    width: 0,
    height: 0,
}
let buffer = {
    width: 0,
    height: 0
}
let canvasElement, canvasContext
function startPlot() {
    state = 1
    canvasElement = document.getElementById("main")
    canvasContext = canvasElement.getContext('2d');
    $.ajax({
        type: "GET",
        url: "http://104.196.99.217:1880/fakeplot",
        success: function (response) {
            console.log("START PLOT")
        }
    });
    const screenRatio = window.innerWidth / window.innerHeight;
    const frameRatio = canvas.width / canvas.height;
    if (frameRatio <= screenRatio) {
        canvasElement.height = 0.9 * window.innerHeight
        canvasElement.width = frameRatio * canvasElement.height
    } else {
        canvasElement.width = 0.95 * window.innerWidth
        canvasElement.height = canvasElement.width / frameRatio
    }
}
function draw(posX, posY, rotateAngle) {
    const realPosX = posX * canvasElement.width / canvas.width
    const realPosY = posY * canvasElement.height / canvas.height
    canvasContext.fillRect(realPosX, realPosY, 1, 1);

}
$(function () {
    $.ajax({
        type: "GET",
        url: "http://104.196.99.217:1880/reset",
        success: function (response) {
            console.log("RESET")
        }
    });
    let wsConnector = new WebSocket(`ws://104.196.99.217:1880/ws`)
    function onWebSocketReceiveMessage(param) {
        const data = JSON.parse(param.data)
        switch (state) {
            case 0:
                if (canvas.width == 0) {
                    buffer.width = data.posX
                    $("#plotWidth").html(data.posX.toFixed(2))
                }
                if (canvas.height == 0) {
                    buffer.height = data.posY
                    $("#plotHeight").html(data.posY.toFixed(2))
                }
                break;
            case 1:
                draw(data.posY, data.posX, data.angleX) // intentionally switch place of param
                $("#posX").html(data.posX.toFixed(2))
                $("#posY").html(data.posY.toFixed(2))
            default:
                break;
        }
    }
    $("#saveCanvasWidth").click(function (e) {
        canvas.width = buffer.width
        if (canvas.height != 0) {
            startPlot();
        }
    });
    $("#saveCanvasHeight").click(function (e) {
        canvas.height = buffer.height
        if (canvas.width != 0) {
            startPlot();
        }
    });
    wsConnector.onmessage = onWebSocketReceiveMessage
});