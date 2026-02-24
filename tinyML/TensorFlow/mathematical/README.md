# 📘 Mathematical Foundation of Backpropagation

This document presents a clean mathematical derivation of the
backpropagation algorithm for fully-connected (dense) neural networks
using vectorized notation.

------------------------------------------------------------------------

## 1️⃣ Notation and Forward Pass

For layer ( l ):

-   Input: ( a\^{(l-1)} `\in `{=tex}`\mathbb{R}`{=tex}\^{n\_{l-1}} )
-   Weights: ( W\^{(l)} `\in `{=tex}`\mathbb{R}`{=tex}\^{n_l
    `\times `{=tex}n\_{l-1}} )
-   Bias: ( b\^{(l)} `\in `{=tex}`\mathbb{R}`{=tex}\^{n_l} )

### Forward equations:

\[ z\^{(l)} = W^{(l)}a^{(l-1)} + b\^{(l)} \]

\[ a\^{(l)} = `\phi`{=tex}^{(l)}(z^{(l)}) \]

For the final layer ( L ):

\[ `\hat{y}`{=tex} = a\^{(L)} \]

Loss function:

\[ `\mathcal{L}`{=tex} = `\mathcal{L}`{=tex}(`\hat{y}`{=tex}, y) \]

------------------------------------------------------------------------

## 2️⃣ Define the Error Signal

Define:

\[ `\delta`{=tex}\^{(l)} =
`\frac{\partial \mathcal{L}}{\partial z^{(l)}}`{=tex} \]

Once computed, gradients are:

\[ `\frac{\partial \mathcal{L}}{\partial W^{(l)}}`{=tex} =
`\delta`{=tex}\^{(l)} (a^{(l-1)})^T \]

\[ `\frac{\partial \mathcal{L}}{\partial b^{(l)}}`{=tex} =
`\delta`{=tex}\^{(l)} \]

------------------------------------------------------------------------

## 3️⃣ Backpropagation Recursion

Using chain rule:

\[ `\delta`{=tex}\^{(l)} = `\left`{=tex}((W^{(l+1)})^T
`\delta`{=tex}\^{(l+1)}`\right`{=tex})
`\odot `{=tex}`\phi`{=tex}'(z\^{(l)}) \]

This is the core recursive equation of backpropagation.

------------------------------------------------------------------------

## 4️⃣ Output Layer Derivations

### A) Mean Squared Error (MSE)

\[ `\mathcal{L}`{=tex} = `\frac{1}{2}`{=tex}\|a\^{(L)} - y\|\^2 \]

\[ `\delta`{=tex}\^{(L)} = (a\^{(L)} - y)
`\odot `{=tex}`\phi`{=tex}'(z\^{(L)}) \]

------------------------------------------------------------------------

### B) Softmax + Cross-Entropy

Softmax:

\[ a_k = `\frac{e^{z_k}}{\sum_j e^{z_j}}`{=tex} \]

Cross-entropy:

\[ `\mathcal{L}`{=tex} = -`\sum`{=tex}\_k y_k `\log`{=tex}(a_k) \]

Key result:

\[ `\delta`{=tex}\^{(L)} = a\^{(L)} - y \]

------------------------------------------------------------------------

## 5️⃣ Mini-Batch Matrix Form

For batch size ( m ):

\[ `\frac{\partial \mathcal{L}}{\partial W^{(l)}}`{=tex} =
`\frac{1}{m}`{=tex} `\Delta`{=tex}\^{(l)} (A^{(l-1)})^T \]

\[ `\frac{\partial \mathcal{L}}{\partial b^{(l)}}`{=tex} =
`\frac{1}{m}`{=tex} `\Delta`{=tex}\^{(l)} `\mathbf{1}`{=tex} \]

Recursive form:

\[ `\Delta`{=tex}\^{(l)} = ((W^{(l+1)})^T `\Delta`{=tex}\^{(l+1)})
`\odot `{=tex}`\phi`{=tex}'(Z\^{(l)}) \]

------------------------------------------------------------------------

## 6️⃣ Activation Derivatives

### ReLU

\[ `\phi`{=tex}'(z) =
```{=tex}
\begin{cases}
1 & z > 0 \\
0 & z \le 0
\end{cases}
```
\]

### Sigmoid

\[ `\sigma`{=tex}'(z) = `\sigma`{=tex}(z)(1-`\sigma`{=tex}(z)) \]

### Tanh

\[ `\tanh`{=tex}'(z) = 1 - `\tanh`{=tex}\^2(z) \]

------------------------------------------------------------------------

## 7️⃣ Backpropagation Algorithm Summary

1.  Forward pass\
2.  Compute output error ( `\delta`{=tex}\^{(L)} )\
3.  Backpropagate errors layer-by-layer\
4.  Compute gradients\
5.  Update parameters (Gradient Descent / Adam / etc.)

------------------------------------------------------------------------

## 8️⃣ Connection to TensorFlow

TensorFlow implements this process using automatic differentiation.

It builds a computation graph and applies the chain rule backward
through all operations to compute gradients efficiently.
