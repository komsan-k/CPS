# Lab 0 — Hello ChatGPT in Node-RED (Beginner Starter)

This lab introduces ChatGPT integration in Node-RED using the OpenAI Responses API.

Flow:
Dashboard Text Input → OpenAI → Dashboard Answer

No ESP32 or hardware is required.

## Requirements
- Node-RED
- node-red-dashboard
- OpenAI API key

## Steps
1. Import the Node-RED flow JSON.
2. Open the function node “Build OpenAI request”.
3. Paste your OpenAI API key.
4. Deploy the flow.
5. Open dashboard: http://<node-red-host>:1880/ui

## Safety Rule (for future CPS labs)
Node-RED controls actuators. ChatGPT explains only.
