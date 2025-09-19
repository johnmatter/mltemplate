// madronalib: a C++ framework for DSP applications.
// Copyright (c) 2020-2022 Madrona Labs LLC. http://www.madronalabs.com
// Distributed under the MIT license: http://madrona-labs.mit-license.org/

// WavetableGen: A time-varying, SIMD-optimized wavetable oscillator for real-time
// quantum simulation and multi-dimensional wavetable synthesis.
// Designed for Schrödinger equation simulation using idiomatic DSPVector operations.
//
// This header is designed to be used within the madronalib build system.
// Include this after the madronalib DSP headers are available.

#pragma once

#include "MLDSPOps.h"
#include "MLDSPGens.h"
#include <functional>
#include <array>
#include <cmath>
#include <complex>

namespace ml
{

// Time-varying wavetable generator
// Supports real-time content updates for simulation
template<size_t TableSizeInVectors = 32>
class TimeVaryingWavetableGen
{
  static constexpr size_t kTableSizeInVectors{TableSizeInVectors};
  static constexpr size_t kTableSizeInSamples{TableSizeInVectors * kFloatsPerDSPVector};
  static constexpr float kTwoPi{6.28318530717958647692f};
  
  // SIMD-optimized wavetable storage - updated each processing cycle
  DSPVectorArray<kTableSizeInVectors> mWavetableReal;
  DSPVectorArray<kTableSizeInVectors> mWavetableImag;
  
  // Oscillator state
  PhasorGen mPhasor;
  
  // Simulation parameters (for Schrödinger equation)
  struct QuantumParams {
    float dt{0.001f};           // Time step
    float dx{1.0f/kTableSizeInSamples}; // Spatial step
    float hbar{1.0f};           // Reduced Planck constant (normalized)
    float mass{1.0f};           // Particle mass (normalized)
  };
  QuantumParams mQuantumParams;
  
public:
  TimeVaryingWavetableGen() 
  {
    // Initialize with Gaussian wave packet (quantum ground state)
    initializeGaussianWavePacket();
  }
  
  ~TimeVaryingWavetableGen() = default;
  
  // Clear phase accumulator and reset wavetable
  void clear() 
  { 
    mPhasor.clear(0);
    initializeGaussianWavePacket();
  }
  
  // Initialize wavetable with a Gaussian wave packet (complex-valued)
  void initializeGaussianWavePacket(float centerPos = 0.5f, float width = 0.1f, float momentum = 0.0f)
  {
    for (size_t vecIdx = 0; vecIdx < kTableSizeInVectors; ++vecIdx)
    {
      DSPVector realVec, imagVec;
      for (int i = 0; i < kFloatsPerDSPVector; ++i)
      {
        float x = static_cast<float>(vecIdx * kFloatsPerDSPVector + i) / kTableSizeInSamples;
        x = (x - centerPos);
        
        // Gaussian wave packet: ψ(x) = exp(-(x/width)²/2) * exp(i*momentum*x)
        float gaussian = std::exp(-(x*x)/(2.0f*width*width));
        realVec[i] = gaussian * std::cos(momentum * x);
        imagVec[i] = gaussian * std::sin(momentum * x);
      }
      mWavetableReal.row(vecIdx) = realVec;
      mWavetableImag.row(vecIdx) = imagVec;
    }
    
    // Normalize the wavefunction
    normalizeWavefunction();
  }
  
  // Set wavetable from complex function (for quantum states)
  void setWavetable(std::function<std::complex<float>(float)> fillFn)
  {
    for (size_t vecIdx = 0; vecIdx < kTableSizeInVectors; ++vecIdx)
    {
      DSPVector realVec, imagVec;
      for (int i = 0; i < kFloatsPerDSPVector; ++i)
      {
        float x = static_cast<float>(vecIdx * kFloatsPerDSPVector + i) / kTableSizeInSamples;
        std::complex<float> value = fillFn(x);
        realVec[i] = value.real();
        imagVec[i] = value.imag();
      }
      mWavetableReal.row(vecIdx) = realVec;
      mWavetableImag.row(vecIdx) = imagVec;
    }
  }
  
  // Set wavetable from real function (imaginary part set to 0)
  void setWavetable(std::function<float(float)> fillFn)
  {
    for (size_t vecIdx = 0; vecIdx < kTableSizeInVectors; ++vecIdx)
    {
      DSPVector realVec, imagVec{0.0f};
      for (int i = 0; i < kFloatsPerDSPVector; ++i)
      {
        float x = static_cast<float>(vecIdx * kFloatsPerDSPVector + i) / kTableSizeInSamples;
        realVec[i] = fillFn(x);
      }
      mWavetableReal.row(vecIdx) = realVec;
      mWavetableImag.row(vecIdx) = imagVec;
    }
  }
  
  // Predefined wavetable generators (real-valued waveforms)
  void setSineWave()
  {
    std::function<float(float)> fillFn = [](float phase) { return std::sin(phase * kTwoPi); };
    setWavetable(fillFn);
  }
  
  void setSawWave()
  {
    std::function<float(float)> fillFn = [](float phase) { return 2.0f * phase - 1.0f; };
    setWavetable(fillFn);
  }
  
  void setTriangleWave()
  {
    std::function<float(float)> fillFn = [](float phase) { 
      return (phase < 0.5f) ? (4.0f * phase - 1.0f) : (3.0f - 4.0f * phase); 
    };
    setWavetable(fillFn);
  }
  
  void setSquareWave()
  {
    std::function<float(float)> fillFn = [](float phase) { 
      return (phase < 0.5f) ? -1.0f : 1.0f; 
    };
    setWavetable(fillFn);
  }
  
  void setPulseWave(float pulseWidth = 0.5f)
  {
    std::function<float(float)> fillFn = [pulseWidth](float phase) { 
      return (phase < pulseWidth) ? 1.0f : -1.0f; 
    };
    setWavetable(fillFn);
  }
  
  // Quantum simulation parameters
  void setQuantumParams(float timeStep, float mass = 1.0f, float hbar = 1.0f)
  {
    mQuantumParams.dt = timeStep;
    mQuantumParams.mass = mass;
    mQuantumParams.hbar = hbar;
  }
  
  // Apply potential energy function to current wavefunction
  void applyPotential(std::function<float(float)> potentialFn, float strength = 1.0f)
  {
    for (size_t vecIdx = 0; vecIdx < kTableSizeInVectors; ++vecIdx)
    {
      DSPVector potential;
      for (int i = 0; i < kFloatsPerDSPVector; ++i)
      {
        float x = static_cast<float>(vecIdx * kFloatsPerDSPVector + i) / kTableSizeInSamples;
        potential[i] = potentialFn(x) * strength * mQuantumParams.dt / mQuantumParams.hbar;
      }
      
      // Apply potential: ψ' = ψ * exp(-i*V*dt/ℏ)
      DSPVector cosV = cos(potential);
      DSPVector sinV = sin(potential);
      
      DSPVector realRow = mWavetableReal.row(vecIdx);
      DSPVector imagRow = mWavetableImag.row(vecIdx);
      
      // Complex multiplication: (a+bi) * (cos-i*sin) = (a*cos+b*sin) + i*(b*cos-a*sin)
      mWavetableReal.row(vecIdx) = realRow * cosV + imagRow * sinV;
      mWavetableImag.row(vecIdx) = imagRow * cosV - realRow * sinV;
    }
  }
  
  // Update wavetable using Schrödinger equation (call once per processing cycle)
  void evolveWavefunction()
  {
    // Apply kinetic energy operator using second derivative (finite difference)
    applyKineticOperator();
  }
  
  // Main operator - generates wavetable output from probability density |ψ|²
  DSPVector operator()(const DSPVector& freq)
  {
    DSPVector phasor = mPhasor(freq);
    return lookupProbabilityDensity(phasor);
  }
  
  // Operator with amplitude modulation
  DSPVector operator()(const DSPVector& freq, const DSPVector& amplitude)
  {
    return operator()(freq) * amplitude;
  }
  
  // Generate output from real part only (for debugging/comparison)
  DSPVector outputRealPart(const DSPVector& freq)
  {
    DSPVector phasor = mPhasor(freq);
    return lookupRealPart(phasor);
  }
  
  // Generate output from imaginary part only
  DSPVector outputImagPart(const DSPVector& freq)
  {
    DSPVector phasor = mPhasor(freq);
    return lookupImagPart(phasor);
  }
  
  // Get current table size
  size_t getTableSizeInSamples() const { return kTableSizeInSamples; }
  size_t getTableSizeInVectors() const { return kTableSizeInVectors; }
  
  // Get probability density at specific position (for visualization)
  float getProbabilityDensityAt(float position) const
  {
    if (position < 0.0f || position >= 1.0f) return 0.0f;
    
    // Simple nearest-neighbor lookup for debugging
    size_t index = static_cast<size_t>(position * kTableSizeInSamples) % kTableSizeInSamples;
    size_t vecIdx = index / kFloatsPerDSPVector;
    size_t elemIdx = index % kFloatsPerDSPVector;
    
    float real = mWavetableReal.constRow(vecIdx)[elemIdx];
    float imag = mWavetableImag.constRow(vecIdx)[elemIdx];
    return real * real + imag * imag;
  }
  
private:
  // SIMD-optimized wavetable lookup methods
  
  // Lookup probability density |ψ|² with linear interpolation
  DSPVector lookupProbabilityDensity(const DSPVector& phase)
  {
    DSPVector result;
    for (int i = 0; i < kFloatsPerDSPVector; ++i)
    {
      float p = phase[i] - std::floor(phase[i]); // Wrap phase to [0, 1)
      float fIndex = p * static_cast<float>(kTableSizeInSamples - 1);
      
      size_t index1 = static_cast<size_t>(fIndex);
      size_t index2 = (index1 + 1) % kTableSizeInSamples;
      float frac = fIndex - std::floor(fIndex);
      
      // Get complex values at both indices
      size_t vecIdx1 = index1 / kFloatsPerDSPVector;
      size_t elemIdx1 = index1 % kFloatsPerDSPVector;
      size_t vecIdx2 = index2 / kFloatsPerDSPVector;
      size_t elemIdx2 = index2 % kFloatsPerDSPVector;
      
      float real1 = mWavetableReal.constRow(vecIdx1)[elemIdx1];
      float imag1 = mWavetableImag.constRow(vecIdx1)[elemIdx1];
      float real2 = mWavetableReal.constRow(vecIdx2)[elemIdx2];
      float imag2 = mWavetableImag.constRow(vecIdx2)[elemIdx2];
      
      // Linear interpolation of complex values
      float realInterp = real1 + frac * (real2 - real1);
      float imagInterp = imag1 + frac * (imag2 - imag1);
      
      // Return probability density |ψ|²
      result[i] = realInterp * realInterp + imagInterp * imagInterp;
    }
    return result;
  }
  
  // Lookup real part with linear interpolation
  DSPVector lookupRealPart(const DSPVector& phase)
  {
    return lookupComponent(phase, mWavetableReal);
  }
  
  // Lookup imaginary part with linear interpolation
  DSPVector lookupImagPart(const DSPVector& phase)
  {
    return lookupComponent(phase, mWavetableImag);
  }
  
  // Generic component lookup with interpolation
  DSPVector lookupComponent(const DSPVector& phase, const DSPVectorArray<kTableSizeInVectors>& component)
  {
    DSPVector result;
    for (int i = 0; i < kFloatsPerDSPVector; ++i)
    {
      float p = phase[i] - std::floor(phase[i]); // Wrap phase to [0, 1)
      float fIndex = p * static_cast<float>(kTableSizeInSamples - 1);
      
      size_t index1 = static_cast<size_t>(fIndex);
      size_t index2 = (index1 + 1) % kTableSizeInSamples;
      float frac = fIndex - std::floor(fIndex);
      
      size_t vecIdx1 = index1 / kFloatsPerDSPVector;
      size_t elemIdx1 = index1 % kFloatsPerDSPVector;
      size_t vecIdx2 = index2 / kFloatsPerDSPVector;
      size_t elemIdx2 = index2 % kFloatsPerDSPVector;
      
      float val1 = component.constRow(vecIdx1)[elemIdx1];
      float val2 = component.constRow(vecIdx2)[elemIdx2];
      
      // Linear interpolation
      result[i] = val1 + frac * (val2 - val1);
    }
    return result;
  }
  
  // Apply kinetic energy operator: -ℏ²/(2m) * ∇²ψ using finite differences
  void applyKineticOperator()
  {
    const float kineticCoeff = -(mQuantumParams.hbar * mQuantumParams.hbar * mQuantumParams.dt) /
                               (2.0f * mQuantumParams.mass * mQuantumParams.dx * mQuantumParams.dx);
    
    // Create temporary arrays for the update
    DSPVectorArray<kTableSizeInVectors> newReal = mWavetableReal;
    DSPVectorArray<kTableSizeInVectors> newImag = mWavetableImag;
    
    // Apply second derivative operator using finite differences: d²ψ/dx² ≈ (ψ[i+1] - 2ψ[i] + ψ[i-1])/dx²
    for (size_t vecIdx = 0; vecIdx < kTableSizeInVectors; ++vecIdx)
    {
      for (int i = 0; i < kFloatsPerDSPVector; ++i)
      {
        size_t currentIdx = vecIdx * kFloatsPerDSPVector + i;
        size_t prevIdx = (currentIdx - 1 + kTableSizeInSamples) % kTableSizeInSamples;
        size_t nextIdx = (currentIdx + 1) % kTableSizeInSamples;
        
        // Get indices in DSPVectorArray format
        size_t prevVecIdx = prevIdx / kFloatsPerDSPVector;
        size_t prevElemIdx = prevIdx % kFloatsPerDSPVector;
        size_t nextVecIdx = nextIdx / kFloatsPerDSPVector;
        size_t nextElemIdx = nextIdx % kFloatsPerDSPVector;
        
        // Get values for finite difference
        float realPrev = mWavetableReal.constRow(prevVecIdx)[prevElemIdx];
        float realCurr = mWavetableReal.constRow(vecIdx)[i];
        float realNext = mWavetableReal.constRow(nextVecIdx)[nextElemIdx];
        
        float imagPrev = mWavetableImag.constRow(prevVecIdx)[prevElemIdx];
        float imagCurr = mWavetableImag.constRow(vecIdx)[i];
        float imagNext = mWavetableImag.constRow(nextVecIdx)[nextElemIdx];
        
        // Second derivative
        float realLaplacian = realNext - 2.0f * realCurr + realPrev;
        float imagLaplacian = imagNext - 2.0f * imagCurr + imagPrev;
        
        // Apply kinetic operator: ψ' = ψ + i*kinetic*∇²ψ*dt
        // Since we want: ψ' = ψ + (i*kinetic*dt)*∇²ψ, and kinetic is already negative:
        DSPVector& realRow = newReal.row(vecIdx);
        DSPVector& imagRow = newImag.row(vecIdx);
        
        realRow[i] = realCurr - kineticCoeff * imagLaplacian;  // Real part gets -i * kinetic * imag_laplacian
        imagRow[i] = imagCurr + kineticCoeff * realLaplacian;  // Imag part gets -i * kinetic * real_laplacian
      }
    }
    
    // Update the wavefunction
    mWavetableReal = newReal;
    mWavetableImag = newImag;
  }
  
  // Normalize the wavefunction so ∫|ψ|²dx = 1
  void normalizeWavefunction()
  {
    // Calculate total probability
    float totalProbability = 0.0f;
    for (size_t vecIdx = 0; vecIdx < kTableSizeInVectors; ++vecIdx)
    {
      DSPVector realVec = mWavetableReal.constRow(vecIdx);
      DSPVector imagVec = mWavetableImag.constRow(vecIdx);
      DSPVector probDensity = realVec * realVec + imagVec * imagVec;
      
      // Sum all elements in this vector
      for (int i = 0; i < kFloatsPerDSPVector; ++i)
      {
        totalProbability += probDensity[i];
      }
    }
    
    // Normalize by dividing by sqrt(total_probability * dx)
    float norm = 1.0f / std::sqrt(totalProbability * mQuantumParams.dx);
    
    for (size_t vecIdx = 0; vecIdx < kTableSizeInVectors; ++vecIdx)
    {
      mWavetableReal.row(vecIdx) = mWavetableReal.row(vecIdx) * DSPVector(norm);
      mWavetableImag.row(vecIdx) = mWavetableImag.row(vecIdx) * DSPVector(norm);
    }
  }
};

// Simple typedef for backward compatibility with existing code
using WavetableGen = TimeVaryingWavetableGen<32>;

} // namespace ml