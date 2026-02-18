// Read previous state
let seenOn  = flow.get("seenOn")  || false;
let seenOff = flow.get("seenOff") || false;

// Check incoming message
let val = String(msg.payload).toUpperCase();

if (val === "ON")  seenOn = true;
if (val === "OFF") seenOff = true;

// Save state
flow.set("seenOn", seenOn);
flow.set("seenOff", seenOff);

// Decide output
if (seenOn && seenOff) {
    msg.payload = "Normal";

    // optional reset for next cycle
    flow.set("seenOn", false);
    flow.set("seenOff", false);

} else {
    msg.payload = "Failure detector";
}

return msg;
