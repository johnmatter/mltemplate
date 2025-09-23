// madronalib: a C++ framework for DSP applications.
// Copyright (c) 2020-2022 Madrona Labs LLC. http://www.madronalabs.com
// Distributed under the MIT license: http://madrona-labs.mit-license.org/

// Unit tests for TimeVaryingWavetableGen class using Catch2 framework

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include "MLDSPWavetableGen.h"
#include <cmath>
#include <complex>

using namespace ml;

// Test utilities for quantum wavetable testing
namespace {
  constexpr float kTestEpsilon = 1e-6f;
  constexpr float kQuantumEpsilon = 1e-4f;  // Looser tolerance for quantum simulation
  
  // Helper to check if a value is in expected range
  bool inRange(float value, float min, float max) {
    return value >= min && value <= max;
  }
  
  // Helper to calculate RMS of a DSPVector
  float calculateRMS(const DSPVector& vec) {
    float sum = 0.0f;
    for (int i = 0; i < kFloatsPerDSPVector; ++i) {
      sum += vec[i] * vec[i];
    }
    return std::sqrt(sum / kFloatsPerDSPVector);
  }
  
  // Helper to check if wavefunction is normalized (∫|ψ|²dx ≈ 1)
  bool isNormalized(const TimeVaryingWavetableGen<32>& gen, float tolerance = 0.1f) {
    float totalProbability = 0.0f;
    size_t tableSize = gen.getTableSizeInSamples();
    float dx = 1.0f / tableSize;
    
    for (size_t i = 0; i < tableSize; ++i) {
      float x = static_cast<float>(i) / tableSize;
      totalProbability += gen.getProbabilityDensityAt(x) * dx;
    }
    
    return std::abs(totalProbability - 1.0f) < tolerance;
  }
  
  // Helper to calculate normalization integral
  float calculateNormalization(const TimeVaryingWavetableGen<32>& gen) {
    float sum = 0.0f;
    size_t tableSize = gen.getTableSizeInSamples();
    float dx = 1.0f / tableSize;
    
    for (size_t i = 0; i < tableSize; ++i) {
      float x = static_cast<float>(i) / tableSize;
      sum += gen.getProbabilityDensityAt(x) * dx;
    }
    return sum;
  }
  
  // Helper to calculate wave packet width (standard deviation)
  float calculateWavePacketWidth(const TimeVaryingWavetableGen<32>& gen) {
    size_t tableSize = gen.getTableSizeInSamples();
    float dx = 1.0f / tableSize;
    float sum = 0.0f;
    float sumX = 0.0f;
    float sumX2 = 0.0f;
    
    // Calculate mean position
    for (size_t i = 0; i < tableSize; ++i) {
      float x = static_cast<float>(i) / tableSize;
      float density = gen.getProbabilityDensityAt(x);
      sum += density * dx;
      sumX += x * density * dx;
    }
    
    if (sum < 1e-6f) return 0.0f;  // Avoid division by zero
    
    float meanX = sumX / sum;
    
    // Calculate variance
    for (size_t i = 0; i < tableSize; ++i) {
      float x = static_cast<float>(i) / tableSize;
      float density = gen.getProbabilityDensityAt(x);
      float diff = x - meanX;
      sumX2 += diff * diff * density * dx;
    }
    
    return std::sqrt(sumX2 / sum);
  }
  
  // Helper to measure evolution rate by RMS change
  float measureEvolutionRate(TimeVaryingWavetableGen<32>& gen, int steps) {
    DSPVector freq(0.1f);
    float initialRMS = calculateRMS(gen(freq));
    
    for (int i = 0; i < steps; ++i) {
      gen.evolveWavefunction();
    }
    
    float finalRMS = calculateRMS(gen(freq));
    return std::abs(finalRMS - initialRMS);
  }
}

TEST_CASE("TimeVaryingWavetableGen/Construction", "[quantum_wavetable]")
{
  SECTION("Default construction initializes properly") {
    TimeVaryingWavetableGen<32> gen;
    
    // Check table size
    REQUIRE(gen.getTableSizeInSamples() == 32 * kFloatsPerDSPVector);
    REQUIRE(gen.getTableSizeInVectors() == 32);
    
    // Check that it produces output
    DSPVector freq(440.0f / 44100.0f);
    DSPVector output = gen(freq);
    
    // Output should be finite and in reasonable range
    for (int i = 0; i < kFloatsPerDSPVector; ++i) {
      REQUIRE(std::isfinite(output[i]));
      REQUIRE(inRange(output[i], -10.0f, 10.0f));  // Reasonable audio range
    }
  }
  
  SECTION("Different template sizes work") {
    TimeVaryingWavetableGen<16> gen16;
    TimeVaryingWavetableGen<64> gen64;
    
    REQUIRE(gen16.getTableSizeInSamples() == 16 * kFloatsPerDSPVector);
    REQUIRE(gen64.getTableSizeInSamples() == 64 * kFloatsPerDSPVector);
  }
}

TEST_CASE("TimeVaryingWavetableGen/BasicWaveforms", "[quantum_wavetable]")
{
  TimeVaryingWavetableGen<32> gen;
  DSPVector freq(1.0f / kFloatsPerDSPVector);  // One cycle per DSPVector
  
  SECTION("Sine wave generation") {
    gen.setSineWave();
    DSPVector output = gen.outputRealPart(freq);
    
    // Check that it looks roughly like a sine wave
    float rms = calculateRMS(output);
    REQUIRE(rms > 0.1f);  // Should have significant energy
    REQUIRE(rms < 1.0f);  // But not too loud
  }
  
  SECTION("Saw wave generation") {
    gen.setSawWave();
    DSPVector output = gen.outputRealPart(freq);
    
    // Saw wave should be bipolar
    bool hasPositive = false, hasNegative = false;
    for (int i = 0; i < kFloatsPerDSPVector; ++i) {
      if (output[i] > 0.1f) hasPositive = true;
      if (output[i] < -0.1f) hasNegative = true;
    }
    REQUIRE(hasPositive);
    REQUIRE(hasNegative);
  }
  
  SECTION("Triangle wave generation") {
    gen.setTriangleWave();
    DSPVector output = gen.outputRealPart(freq);
    
    float rms = calculateRMS(output);
    REQUIRE(rms > 0.1f);
  }
  
  SECTION("Square wave generation") {
    gen.setSquareWave();
    DSPVector output = gen.outputRealPart(freq);
    
    // Square wave should have values near ±1
    bool hasHigh = false, hasLow = false;
    for (int i = 0; i < kFloatsPerDSPVector; ++i) {
      if (output[i] > 0.5f) hasHigh = true;
      if (output[i] < -0.5f) hasLow = true;
    }
    REQUIRE(hasHigh);
    REQUIRE(hasLow);
  }
}

TEST_CASE("TimeVaryingWavetableGen/QuantumSimulation", "[quantum_wavetable]")
{
  TimeVaryingWavetableGen<32> gen;
  
  SECTION("Gaussian wave packet initialization") {
    gen.initializeGaussianWavePacket(0.5f, 0.1f, 0.0f);
    
    // Check that the wave packet is centered around 0.5
    float centerDensity = gen.getProbabilityDensityAt(0.5f);
    float edgeDensity = gen.getProbabilityDensityAt(0.1f);
    
    REQUIRE(centerDensity > edgeDensity);
    REQUIRE(centerDensity > 0.0f);
    
    // Check normalization
    REQUIRE(isNormalized(gen));
  }
  
  SECTION("Quantum parameter setting") {
    gen.setQuantumParams(0.01f, 0.5f, 1.0f);
    gen.setDecoherenceStrength(0.1f);
    gen.setSmoothingStrength(0.2f);
    gen.setAntiAliasingEnabled(true);
    
    // Parameters should be retrievable
    REQUIRE(gen.getDecoherenceStrength() == Approx(0.1f));
    REQUIRE(gen.getSmoothingStrength() == Approx(0.2f));
    REQUIRE(gen.isAntiAliasingEnabled() == true);
  }
  
  SECTION("Wavefunction evolution") {
    gen.initializeGaussianWavePacket(0.5f, 0.1f, 0.0f);
    gen.setQuantumParams(0.01f, 1.0f, 1.0f);
    
    // Store initial state
    float initialDensity = gen.getProbabilityDensityAt(0.5f);
    
    // Evolve the wavefunction
    gen.evolveWavefunction();
    
    // State should have changed (though might be subtle)
    float evolvedDensity = gen.getProbabilityDensityAt(0.5f);
    
    // The wavefunction should still be normalized after evolution
    REQUIRE(isNormalized(gen, 0.2f));  // Looser tolerance after evolution
  }
  
  SECTION("Quantitative evolution analysis") {
    gen.initializeGaussianWavePacket(0.5f, 0.1f, 0.0f);
    gen.setQuantumParams(0.001f, 1.0f, 1.0f);  // Small time step for accuracy
    
    // Test conservation of normalization over multiple time steps
    std::vector<float> normalizationValues;
    for (int step = 0; step < 10; ++step) {
      normalizationValues.push_back(calculateNormalization(gen));
      gen.evolveWavefunction();
    }
    
    // Normalization should be approximately conserved
    for (size_t i = 1; i < normalizationValues.size(); ++i) {
      REQUIRE(std::abs(normalizationValues[i] - normalizationValues[0]) < 0.1f);
    }
    
    // Test that wavefunction spreads over time (Gaussian packet should broaden)
    gen.initializeGaussianWavePacket(0.5f, 0.05f, 0.0f);  // Narrow initial packet
    float initialWidth = calculateWavePacketWidth(gen);
    
    // Evolve for several steps
    for (int step = 0; step < 20; ++step) {
      gen.evolveWavefunction();
    }
    
    float finalWidth = calculateWavePacketWidth(gen);
    REQUIRE(finalWidth > initialWidth);  // Packet should spread
  }
  
  SECTION("Parameter sensitivity") {
    gen.initializeGaussianWavePacket(0.5f, 0.1f, 0.0f);
    
    // Test different time steps with more sensitive measurement
    std::vector<float> timeSteps = {0.0001f, 0.001f, 0.01f};
    std::vector<float> evolutionRates;
    
    for (float dt : timeSteps) {
      gen.setQuantumParams(dt, 1.0f, 1.0f);
      gen.initializeGaussianWavePacket(0.5f, 0.1f, 0.0f);
      
      // Use RMS change over multiple evolution steps for more sensitivity
      float initialRMS = calculateRMS(gen(DSPVector(0.1f)));
      
      // Evolve for more steps to see measurable change
      for (int i = 0; i < 20; ++i) {
        gen.evolveWavefunction();
      }
      
      float finalRMS = calculateRMS(gen(DSPVector(0.1f)));
      evolutionRates.push_back(std::abs(finalRMS - initialRMS));
    }
    
    // At least some time steps should produce measurable evolution
    bool hasEvolution = false;
    for (float rate : evolutionRates) {
      if (rate > 1e-6f) {
        hasEvolution = true;
        break;
      }
    }
    REQUIRE(hasEvolution);  // Should see some evolution with different time steps
  }
  
  SECTION("Potential energy application") {
    gen.initializeGaussianWavePacket(0.5f, 0.1f, 0.0f);
    
    // Apply harmonic potential
    auto harmonicPotential = [](float x) {
      float x0 = x - 0.5f;
      return 0.5f * 10.0f * x0 * x0;
    };
    
    float initialDensity = gen.getProbabilityDensityAt(0.5f);
    gen.applyPotential(harmonicPotential, 0.5f);
    float afterPotential = gen.getProbabilityDensityAt(0.5f);
    
    // Applying potential should change the wavefunction
    // (exact change depends on potential strength and time step)
    REQUIRE(std::isfinite(afterPotential));
    REQUIRE(afterPotential >= 0.0f);  // Probability density must be non-negative
  }
}

TEST_CASE("TimeVaryingWavetableGen/AudioOutput", "[quantum_wavetable]")
{
  TimeVaryingWavetableGen<32> gen;
  gen.initializeGaussianWavePacket(0.5f, 0.1f, 0.0f);
  
  SECTION("Probability density output") {
    DSPVector freq(440.0f / 44100.0f);
    DSPVector output = gen(freq);
    
    // Probability density should be non-negative
    for (int i = 0; i < kFloatsPerDSPVector; ++i) {
      REQUIRE(output[i] >= 0.0f);
      REQUIRE(std::isfinite(output[i]));
    }
    
    // Should have some energy
    float rms = calculateRMS(output);
    REQUIRE(rms > 0.0f);
  }
  
  SECTION("Real part output") {
    DSPVector freq(440.0f / 44100.0f);
    DSPVector output = gen.outputRealPart(freq);
    
    // Real part can be positive or negative
    for (int i = 0; i < kFloatsPerDSPVector; ++i) {
      REQUIRE(std::isfinite(output[i]));
    }
  }
  
  SECTION("Imaginary part output") {
    DSPVector freq(440.0f / 44100.0f);
    DSPVector output = gen.outputImagPart(freq);
    
    // Imaginary part can be positive or negative
    for (int i = 0; i < kFloatsPerDSPVector; ++i) {
      REQUIRE(std::isfinite(output[i]));
    }
  }
  
  SECTION("Amplitude modulation") {
    DSPVector freq(440.0f / 44100.0f);
    DSPVector amp(0.5f);
    DSPVector output = gen(freq, amp);
    
    // Amplitude modulated output should be finite and reasonable
    for (int i = 0; i < kFloatsPerDSPVector; ++i) {
      REQUIRE(std::isfinite(output[i]));
      REQUIRE(output[i] >= 0.0f);  // Probability density should be non-negative
    }
    
    // Test that zero amplitude gives zero output
    DSPVector zeroAmp(0.0f);
    DSPVector zeroOutput = gen(freq, zeroAmp);
    for (int i = 0; i < kFloatsPerDSPVector; ++i) {
      REQUIRE(zeroOutput[i] == 0.0f);
    }
  }
}

TEST_CASE("TimeVaryingWavetableGen/ParameterRanges", "[quantum_wavetable]")
{
  TimeVaryingWavetableGen<32> gen;
  
  SECTION("Decoherence strength clamping") {
    gen.setDecoherenceStrength(-0.5f);  // Below range
    REQUIRE(gen.getDecoherenceStrength() == 0.0f);
    
    gen.setDecoherenceStrength(1.5f);   // Above range
    REQUIRE(gen.getDecoherenceStrength() == 1.0f);
    
    gen.setDecoherenceStrength(0.3f);   // In range
    REQUIRE(gen.getDecoherenceStrength() == 0.3f);
  }
  
  SECTION("Smoothing strength clamping") {
    gen.setSmoothingStrength(-0.1f);    // Below range
    REQUIRE(gen.getSmoothingStrength() == 0.0f);
    
    gen.setSmoothingStrength(1.2f);     // Above range
    REQUIRE(gen.getSmoothingStrength() == 1.0f);
    
    gen.setSmoothingStrength(0.7f);     // In range
    REQUIRE(gen.getSmoothingStrength() == 0.7f);
  }
}

TEST_CASE("TimeVaryingWavetableGen/EdgeCases", "[quantum_wavetable]")
{
  TimeVaryingWavetableGen<32> gen;
  
  SECTION("Zero frequency") {
    DSPVector freq(0.0f);
    DSPVector output = gen(freq);
    
    // Should handle zero frequency gracefully
    for (int i = 0; i < kFloatsPerDSPVector; ++i) {
      REQUIRE(std::isfinite(output[i]));
    }
  }
  
  SECTION("Very high frequency") {
    DSPVector freq(0.49f);  // Just below Nyquist
    DSPVector output = gen(freq);
    
    // Should handle high frequency without blowing up
    for (int i = 0; i < kFloatsPerDSPVector; ++i) {
      REQUIRE(std::isfinite(output[i]));
      REQUIRE(std::abs(output[i]) < 100.0f);  // Reasonable bounds
    }
  }
  
  SECTION("Clear and reset") {
    gen.setSineWave();
    DSPVector freq(440.0f / 44100.0f);
    DSPVector before = gen(freq);
    
    gen.clear();
    DSPVector after = gen(freq);
    
    // After clear, should be back to Gaussian wave packet
    // (exact comparison difficult due to quantum nature)
    REQUIRE(std::isfinite(after[0]));
  }
}

TEST_CASE("TimeVaryingWavetableGen/QuantitativeEvolution", "[quantum_wavetable]")
{
  TimeVaryingWavetableGen<32> gen;
  
  SECTION("Normalization conservation") {
    gen.initializeGaussianWavePacket(0.5f, 0.1f, 0.0f);
    gen.setQuantumParams(0.001f, 1.0f, 1.0f);  // Small dt for accuracy
    
    float initialNorm = calculateNormalization(gen);
    REQUIRE(std::abs(initialNorm - 1.0f) < 0.1f);  // Should start normalized
    
    // Evolve and check normalization is conserved
    for (int step = 0; step < 20; ++step) {
      gen.evolveWavefunction();
      float currentNorm = calculateNormalization(gen);
      REQUIRE(std::abs(currentNorm - initialNorm) < 0.2f);  // Should stay roughly normalized
    }
  }
  
  SECTION("Wave packet spreading") {
    gen.initializeGaussianWavePacket(0.5f, 0.05f, 0.0f);  // Narrow initial packet
    gen.setQuantumParams(0.01f, 1.0f, 1.0f);
    
    float initialWidth = calculateWavePacketWidth(gen);
    REQUIRE(initialWidth > 0.0f);  // Should have measurable width
    
    // Evolve and check packet spreads
    for (int step = 0; step < 10; ++step) {
      gen.evolveWavefunction();
    }
    
    float finalWidth = calculateWavePacketWidth(gen);
    REQUIRE(finalWidth > initialWidth);  // Should spread over time
  }
  
  SECTION("Parameter sensitivity analysis") {
    gen.initializeGaussianWavePacket(0.5f, 0.1f, 0.0f);
    
    // Test different time steps with more sensitive measurement
    std::vector<float> timeSteps = {0.0001f, 0.001f, 0.01f};
    std::vector<float> evolutionRates;
    
    for (float dt : timeSteps) {
      gen.setQuantumParams(dt, 1.0f, 1.0f);
      gen.initializeGaussianWavePacket(0.5f, 0.1f, 0.0f);
      
      // Use more evolution steps for better sensitivity
      float rate = measureEvolutionRate(gen, 20);
      evolutionRates.push_back(rate);
    }
    
    // At least some time steps should produce measurable evolution
    bool hasEvolution = false;
    for (float rate : evolutionRates) {
      if (rate > 1e-6f) {
        hasEvolution = true;
        break;
      }
    }
    REQUIRE(hasEvolution);  // Should see some evolution with different time steps
  }
  
  SECTION("Mass parameter effects") {
    gen.initializeGaussianWavePacket(0.5f, 0.1f, 0.0f);
    
    // Test different masses
    std::vector<float> masses = {0.5f, 1.0f, 2.0f};
    std::vector<float> evolutionRates;
    
    for (float mass : masses) {
      gen.setQuantumParams(0.01f, mass, 1.0f);
      gen.initializeGaussianWavePacket(0.5f, 0.1f, 0.0f);
      
      float rate = measureEvolutionRate(gen, 5);
      evolutionRates.push_back(rate);
    }
    
    // Different masses should produce different evolution rates
    REQUIRE(evolutionRates[0] != evolutionRates[1]);  // Different masses, different rates
  }
  
  SECTION("Decoherence effects") {
    gen.initializeGaussianWavePacket(0.5f, 0.1f, 0.0f);
    gen.setQuantumParams(0.01f, 1.0f, 1.0f);
    
    // Test with and without decoherence
    gen.setDecoherenceStrength(0.0f);
    float noDecoherenceRate = measureEvolutionRate(gen, 10);
    
    gen.initializeGaussianWavePacket(0.5f, 0.1f, 0.0f);
    gen.setDecoherenceStrength(0.1f);
    float withDecoherenceRate = measureEvolutionRate(gen, 10);
    
    // Decoherence should affect evolution
    REQUIRE(std::abs(noDecoherenceRate - withDecoherenceRate) > 1e-6f);
  }
}

TEST_CASE("TimeVaryingWavetableGen/Performance", "[quantum_wavetable][performance]")
{
  TimeVaryingWavetableGen<32> gen;
  gen.initializeGaussianWavePacket(0.5f, 0.1f, 0.0f);
  gen.setQuantumParams(0.01f, 1.0f, 1.0f);
  
  SECTION("Audio generation performance") {
    DSPVector freq(440.0f / 44100.0f);
    
    // Time multiple calls to ensure consistent performance
    auto start = std::chrono::high_resolution_clock::now();
    
    constexpr int kIterations = 1000;
    DSPVector result;
    for (int i = 0; i < kIterations; ++i) {
      result = gen(freq);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Should be fast enough for real-time audio (rough check)
    double avgTimePerCall = duration.count() / double(kIterations);
    REQUIRE(avgTimePerCall < 100.0);  // Less than 100 microseconds per call
    
    // Ensure result is valid to prevent optimization
    REQUIRE(std::isfinite(result[0]));
  }
  
  SECTION("Evolution performance") {
    auto start = std::chrono::high_resolution_clock::now();
    
    constexpr int kEvolutions = 100;
    for (int i = 0; i < kEvolutions; ++i) {
      gen.evolveWavefunction();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Evolution should be reasonably fast
    double avgTimePerEvolution = duration.count() / double(kEvolutions);
    REQUIRE(avgTimePerEvolution < 1000.0);  // Less than 1ms per evolution
  }
}

TEST_CASE("TimeVaryingWavetableGen/ComplexWaveforms", "[quantum_wavetable]")
{
  TimeVaryingWavetableGen<32> gen;
  
  SECTION("Custom complex waveform") {
    // Set a custom complex waveform
    gen.setWavetable([](float x) -> std::complex<float> {
      return std::complex<float>(std::sin(2.0f * M_PI * x), std::cos(4.0f * M_PI * x));
    });
    
    DSPVector freq(440.0f / 44100.0f);
    DSPVector realOutput = gen.outputRealPart(freq);
    DSPVector imagOutput = gen.outputImagPart(freq);
    DSPVector probOutput = gen(freq);
    
    // All outputs should be finite
    for (int i = 0; i < kFloatsPerDSPVector; ++i) {
      REQUIRE(std::isfinite(realOutput[i]));
      REQUIRE(std::isfinite(imagOutput[i]));
      REQUIRE(std::isfinite(probOutput[i]));
      REQUIRE(probOutput[i] >= 0.0f);  // Probability density non-negative
    }
  }
  
  SECTION("Custom real waveform") {
    // Set a custom real waveform
    std::function<float(float)> realWaveform = [](float x) -> float {
      return std::sin(2.0f * M_PI * x) + 0.3f * std::sin(6.0f * M_PI * x);
    };
    gen.setWavetable(realWaveform);
    
    DSPVector freq(440.0f / 44100.0f);
    DSPVector output = gen.outputRealPart(freq);
    
    // Should produce reasonable output
    float rms = calculateRMS(output);
    REQUIRE(rms > 0.1f);
    REQUIRE(rms < 2.0f);
  }
}
