# 🤖 TinyML Neural Network for ESP32 + LDR (Plain Arduino Implementation)

## Overview

This guide explains how to build a TinyML Neural Network for an LDR
using an ESP32. Instead of fixed threshold logic, the ESP32 learns light
patterns such as SUNNY, OFFICE, and NIGHT.

Advantages: - Ultra-lightweight memory usage - Fast execution - Ideal
for CPS and Edge AI education

------------------------------------------------------------------------

# 🧩 Hardware Setup

Voltage Divider Circuit:

3.3V → LDR\
LDR → GPIO36 (ADC)\
LDR → 10kΩ → GND

------------------------------------------------------------------------

# 🧠 TinyML Workflow

## Data Collection

Collect LDR readings and label them:

Label,Value\
0,4095\
1,1500\
2,200

Normalize input:

input = analogRead / 4095.0

------------------------------------------------------------------------

## Neural Network Architecture

Input Layer: 1 node\
Hidden Layer: 8 neurons (ReLU)\
Output Layer: 3 neurons

------------------------------------------------------------------------

# ⚡ Plain Arduino TinyML Code (ESP32)

``` cpp
#include <math.h>

const int LDR_PIN = 36;

float w1[8] = {-0.5,0.8,1.2,-0.3,0.6,-1.1,0.4,0.9};
float b1[8] = {0.1,-0.1,0.2,0.0,0.1,-0.2,0.1,0.1};

float w2[3][8] = {
 {0.5,-0.2,0.8,-0.1,0.4,-0.6,0.2,0.1},
 {-0.1,0.6,-0.2,0.9,-0.3,0.2,0.5,-0.4},
 {-0.4,-0.5,-0.1,-0.2,0.1,0.9,-0.7,0.8}
};

void setup(){
 Serial.begin(115200);
}

void loop(){

 float input = 1.0 - (analogRead(LDR_PIN)/4095.0);

 float hidden[8];
 for(int i=0;i<8;i++){
   float sum = input*w1[i] + b1[i];
   hidden[i] = (sum>0)?sum:0;
 }

 float scores[3]={0,0,0};
 for(int i=0;i<3;i++){
   for(int j=0;j<8;j++){
     scores[i]+=hidden[j]*w2[i][j];
   }
 }

 int prediction=0;
 float max_score=scores[0];
 if(scores[1]>max_score){prediction=1;max_score=scores[1];}
 if(scores[2]>max_score){prediction=2;}

 const char* labels[]={"SUNNY","OFFICE","NIGHT"};
 Serial.println(labels[prediction]);

 delay(500);
}
```

------------------------------------------------------------------------

# 🚀 Why Plain TinyML Instead of TensorFlow Lite?

Memory Usage: Very Low\
Speed: Fast\
Libraries: None required

------------------------------------------------------------------------

# 📡 CPS Pipeline Concept

Sense → Normalize → Neural Network → Predict → Act

------------------------------------------------------------------------

License: Educational use recommended.
