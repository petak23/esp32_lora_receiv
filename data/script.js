var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onLoad);

function mqttStatus(id) {
  return parseInt(id) == 1 ? "MQTT ready" : "MQTT_DISCONNECTED - the client is disconnected...";
}

function onLoad(event) {
  initWebSocket();
}

function initWebSocket() {
  console.log('Trying to open a WebSocket connection...');
  websocket = new WebSocket(gateway);
  websocket.onopen    = onOpen;
  websocket.onclose   = onClose;
  websocket.onmessage = onMessage;
}

function onOpen(event) {
  console.log('Connection opened');
  websocket.send("states");
}
  
function onClose(event) {
  console.log('Connection closed');
  setTimeout(initWebSocket, 2000);
} 

function onMessage(event) {
  let myObj = JSON.parse(event.data);
  console.log(myObj);

  /* Mqtt state */
  let mq = document.getElementById("mqtt");
  mq.innerHTML = mqttStatus(myObj.mqtt);
  mq.classList.remove("alert-success", "alert-danger");
  mq.classList.add((myObj.mqtt == 1 ? "alert-success" : "alert-danger"));

  let lora = myObj.lora.split(";"); // Rozdelenie meraných veličín do poľa
  const units = {"TE": "°C", "HU": "%", "AP": "hPa", "RP": "hPa", "AL": "m.n.m"}
  
  for (i in lora) {
    let out = lora[i].split(":"); // Rozdelenie na názov a hodnotu
    let st = document.getElementById(out[0]);
    st.innerHTML = out[1] + " " + units[out[0]];
  }
  let rssi = document.getElementById("RSSI");
  console.log(rssi);
  rssi.innerHTML = myObj.rssi + " dBm (" + parseInt((10 / 9) * (myObj.rssi + 120)) + "%)";

  let last_time = document.getElementById("last_time");
  const d = new Date();
  last_time.innerHTML = d.getDay() + "." + d.getMonth() + "." + d.getFullYear() + " " + d.getHours() + ":" + d.getMinutes() + ":" + d.getSeconds();
}