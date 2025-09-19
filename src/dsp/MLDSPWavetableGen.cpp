// madronalib: a C++ framework for DSP applications.
// Copyright (c) 2020-2022 Madrona Labs LLC. http://www.madronalabs.com
// Distributed under the MIT license: http://madrona-labs.mit-license.org/

#include "MLDSPWavetableGen.h"

namespace ml
{

// Example usage: Quantum harmonic oscillator simulation
//
// void quantumHarmonicOscillatorExample()
// {
//   TimeVaryingWavetableGen<32> quantumOsc;
//   
//   // Set up quantum parameters
//   quantumOsc.setQuantumParams(0.001f, 1.0f, 1.0f); // dt, mass, hbar
//   
//   // Initialize with ground state wave packet
//   quantumOsc.initializeGaussianWavePacket(0.5f, 0.1f, 0.0f);
//   
//   // Define harmonic potential V(x) = ½kx² centered at x=0.5
//   auto harmonicPotential = [](float x) {
//     float x0 = x - 0.5f;  // Center at x=0.5
//     return 0.5f * 10.0f * x0 * x0;  // k=10 spring constant
//   };
//   
//   // In your audio processing loop:
//   // 1. Apply potential for half time step
//   quantumOsc.applyPotential(harmonicPotential, 0.5f);
//   
//   // 2. Apply kinetic energy (full time step) - evolves the wavefunction
//   quantumOsc.evolveWavefunction();
//   
//   // 3. Apply potential for remaining half time step (split-operator method)
//   quantumOsc.applyPotential(harmonicPotential, 0.5f);
//   
//   // 4. Generate audio from probability density
//   DSPVector frequency = DSPVector(440.0f / sampleRate);  // A4 note
//   DSPVector audioOutput = quantumOsc(frequency);
//   
//   // Alternative outputs:
//   // DSPVector realPart = quantumOsc.outputRealPart(frequency);
//   // DSPVector imagPart = quantumOsc.outputImagPart(frequency);
// }

// The TimeVaryingWavetableGen is fully header-only (template-based)
// No implementation needed in .cpp file

} // namespace ml
