let value = Number(msg.payload);

if (!isNaN(value) && value < 280) {
    msg.payload = "ON";
} else {
    msg.payload = "OFF";
}

return msg;
