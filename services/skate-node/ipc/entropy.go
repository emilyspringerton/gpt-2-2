package ipc

import (
	"math"
)

// EntropyValidator analyzes input patterns to detect scripts/bots
type EntropyValidator struct {
	history []float32
}

func NewEntropyValidator() *EntropyValidator {
	return &EntropyValidator{
		history: make([]float32, 0, 60),
	}
}

// Validate checks if the claimed entropy is plausible given the hash history
// In a real implementation, we would need the raw input stream to do FFT.
// Here we check statistical anomalies in the *reported* entropy.
func (e *EntropyValidator) Validate(claimedEntropy float32, frameVar float32) float32 {
	// 1. Detect Static Input (AFK)
	if claimedEntropy < 0.01 {
		return 0.0
	}

	// 2. Detect Perfectly Constant Frame Times (Virtual Display / Limiter)
	if frameVar < 0.0001 {
		// Suspiciously stable framerate
		return 0.0
	}

	// 3. History Analysis (Detect Periodicity)
	e.history = append(e.history, claimedEntropy)
	if len(e.history) > 60 {
		e.history = e.history[1:]
	}

	if e.detectPeriodicity() {
		return 0.0 // Bot script detected
	}

	return claimedEntropy
}

// Simple heuristic to detect repetitive entropy scores (e.g. macro loops)
func (e *EntropyValidator) detectPeriodicity() bool {
	if len(e.history) < 10 {
		return false
	}
	
	// Check for repeating patterns of length 2, 3, 4
	// Very naive O(N^2) check, but N is small (60)
	for period := 2; period < 10; period++ {
		matches := 0
		for i := 0; i < len(e.history)-period; i++ {
			if math.Abs(float64(e.history[i] - e.history[i+period])) < 0.001 {
				matches++
			}
		}
		// If > 80% of samples match a periodic pattern
		if float64(matches)/float64(len(e.history)) > 0.8 {
			return true
		}
	}
	return false
}
