# 🧠 Neural Network Architecture

A **neural network architecture** defines how artificial neurons are
organized, connected, and how data flows through the system to perform
tasks such as classification, regression, prediction, or control.

Understanding architecture is critical in: - Cyber-Physical Systems
(CPS) - TinyML and Edge AI - Digital Twins - IoT-based intelligent
systems

------------------------------------------------------------------------

# 1️⃣ Basic Feedforward Neural Network (FNN / MLP)

## Structure

A standard **Feedforward Neural Network (FNN)**, also known as a
**Multilayer Perceptron (MLP)**, consists of:

1.  Input Layer
2.  Hidden Layer(s)
3.  Output Layer

Data flows in one direction:

    Input → Hidden Layers → Output

------------------------------------------------------------------------

## Mathematical Formulation

For layer ( l ):

$$
z\^(l) = W\^(l) a\^(l-1) + b\^(l)\
$$

$$
a\^(l) = σ(z\^(l))
$$

Where:

-   W = weight matrix
-   b = bias vector
-   σ = activation function
-   a = layer output

------------------------------------------------------------------------

## Activation Functions

Common activation functions:

-   ReLU
-   Sigmoid
-   Tanh
-   Softmax (for classification)

------------------------------------------------------------------------

# 2️⃣ Deep Neural Network (DNN)

A neural network becomes **deep** when it contains multiple hidden
layers.

### Advantages

-   Learns hierarchical features
-   Better nonlinear modeling
-   Suitable for CPS prediction and digital twins

### Disadvantages

-   Higher computation
-   Risk of overfitting
-   Higher energy consumption (important for edge devices)

------------------------------------------------------------------------

# 3️⃣ Convolutional Neural Network (CNN)

Used mainly for:

-   Image processing
-   Spatial signals
-   Vibration spectrograms (CPS fault detection)
-   Smart camera systems

### Main Components

1.  Convolution layer
2.  Activation
3.  Pooling
4.  Fully connected layer

CNN reduces parameter count compared to fully connected networks.

------------------------------------------------------------------------

# 4️⃣ Recurrent Neural Network (RNN)

Used for:

-   Time-series prediction
-   CPS forecasting
-   IoT sensor trend analysis
-   Traffic prediction

RNN includes feedback connections:
$$
h_t = f(Wx_t + Uh\_(t-1))
$$
### Variants

-   LSTM
-   GRU

Suitable for sequential data such as: - Temperature trends
- Lux prediction
- Network traffic behavior

------------------------------------------------------------------------

# 5️⃣ Transformer Architecture

Used in:

-   Large Language Models
-   Advanced CPS cognition
-   Edge-cloud hybrid intelligence

Core idea: - Self-attention mechanism
- Parallel processing (no recurrence)

------------------------------------------------------------------------

# 6️⃣ Neural Networks in CPS 5C Context

  CPS Layer       Neural Network Role      Example Architecture
  --------------- ------------------------ ------------------------
  Connection      Preprocessing            Small MLP
  Conversion      Feature extraction       CNN / Autoencoder
  Cyber           Modeling                 DNN
  Cognition       Decision-making          RNN / Transformer
  Configuration   Optimization & Control   Reinforcement Learning

------------------------------------------------------------------------

# 7️⃣ Architecture Design Trade-Off

  Factor               Shallow NN   Deep NN
  -------------------- ------------ -----------
  Accuracy             Moderate     High
  Computation          Low          High
  Energy Usage         Low          High
  Suitable for ESP32   Yes          Limited
  Cloud Deployment     Limited      Excellent

------------------------------------------------------------------------

# 8️⃣ Example: TinyML Neural Network for ESP32

Example architecture:

Input: Lux value (1 feature)
Hidden layer: 8 neurons
Output: 3 classes

Architecture:

1 → 8 → 3

Parameter count:

(1×8 + 8) + (8×3 + 3) = 43 parameters

Lightweight and ideal for Edge CPS applications.

------------------------------------------------------------------------

# 9️⃣ Neural Network Training Pipeline

    Data Acquisition
            ↓
    Preprocessing
            ↓
    Feature Extraction
            ↓
    Neural Network Layers
            ↓
    Loss Function
            ↓
    Backpropagation
            ↓
    Optimization

------------------------------------------------------------------------

# 🔟 Summary

A neural network architecture defines:

-   Number of layers
-   Number of neurons
-   Type of connections
-   Activation functions
-   Flow of data

Different architectures serve different purposes:

-   MLP → Simple prediction
-   CNN → Spatial sensing
-   RNN → Temporal prediction
-   Transformer → Advanced cognition
-   RL → Adaptive configuration

