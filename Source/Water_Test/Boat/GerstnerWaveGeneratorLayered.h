#pragma once

#include "CoreMinimal.h"
#include "GerstnerWaterWaves.h"
#include "GerstnerWaveGeneratorLayered.generated.h"

/**
 * Configuration for one spectral band in the layered wave generator.
 * Each layer runs its own independent random stream so bands never interfere
 * with each other's distribution, and any layer can be tweaked or disabled
 * without affecting the others.
 */
USTRUCT(BlueprintType)
struct FWaveLayerConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layer")
	bool bEnabled = true;

	/** Number of Gerstner waves generated in this band. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layer",
		meta=(UIMin=1, ClampMin=1, UIMax=64, ClampMax=256))
	int32 NumWaves = 8;

	/**
	 * Added to MasterSeed to produce this layer's unique random stream.
	 * Keep offsets far apart (e.g. 0, 100, 200, 300) so layers don't correlate.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layer")
	int32 SeedOffset = 0;

	/** Runtime scale applied to all amplitudes in this layer.
	 *  Use to fade a band in/out (e.g. calm-weather dial-down of chop). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layer",
		meta=(UIMin=0, ClampMin=0, UIMax=2))
	float AmplitudeScale = 1.f;

	// ---- Direction -----------------------------------------------------------

	/** Primary propagation direction of this wave band, in degrees. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Direction",
		meta=(Units="deg", UIMin=-180, ClampMin=-180, UIMax=180, ClampMax=180))
	float WindAngleDeg = 0.f;

	/**
	 * Half-angle of the directional cone around WindAngleDeg.
	 * Waves are spread randomly within ±DirectionSpreadDeg.
	 * The dominant (first) wave always travels exactly at WindAngleDeg.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Direction",
		meta=(Units="deg", UIMin=0, ClampMin=0, UIMax=180))
	float DirectionSpreadDeg = 30.f;

	// ---- Wavelength ----------------------------------------------------------

	/** Shortest wavelength generated in this band (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Wavelength",
		meta=(UIMin=1, ClampMin=1, Units="cm"))
	float MinWavelength = 500.f;

	/** Longest wavelength generated in this band (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Wavelength",
		meta=(UIMin=1, ClampMin=1, Units="cm"))
	float MaxWavelength = 6000.f;

	/** Power-law exponent biasing distribution toward longer wavelengths.
	 *  Higher = more energy at the long end of the band. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Wavelength",
		meta=(UIMin=0, ClampMin=0, UIMax=8))
	float WavelengthFalloff = 2.f;

	// ---- Amplitude -----------------------------------------------------------

	/** Smallest amplitude in this band (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Amplitude",
		meta=(UIMin=0.0001f, ClampMin=0.0001f, Units="cm"))
	float MinAmplitude = 4.f;

	/** Largest amplitude in this band (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Amplitude",
		meta=(UIMin=0.0001f, ClampMin=0.0001f, Units="cm"))
	float MaxAmplitude = 80.f;

	/** Power-law exponent biasing distribution toward larger amplitudes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Amplitude",
		meta=(UIMin=0, ClampMin=0, UIMax=8))
	float AmplitudeFalloff = 2.f;

	// ---- Steepness -----------------------------------------------------------

	/** Steepness of the longest (dominant) wave in this band. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Steepness",
		meta=(UIMin=0, ClampMin=0, UIMax=1, ClampMax=1))
	float LargeSteepness = 0.2f;

	/** Steepness of the shortest wave in this band. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Steepness",
		meta=(UIMin=0, ClampMin=0, UIMax=1, ClampMax=1))
	float SmallSteepness = 0.4f;

	/** Power-law exponent on the steepness ramp from large to small. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Steepness",
		meta=(UIMin=0, ClampMin=0, UIMax=8))
	float SteepnessFalloff = 1.f;

	// ---- Distribution jitter -------------------------------------------------

	/** How much random noise is added to each wave's Alpha index position.
	 *  0 = perfectly uniform distribution; 1 = fully random within the band. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layer",
		meta=(UIMin=0, ClampMin=0, UIMax=2))
	float Randomness = 0.5f;
};


/**
 * Multi-layered Gerstner wave generator.
 *
 * Produces distinct spectral bands (swell, secondary swell, chop, ripple)
 * as separate independent layers, each with its own direction, wavelength
 * range, amplitude range, and steepness.  Layers are concatenated into the
 * same FGerstnerWave array that UGerstnerWaterWaves consumes, so no engine
 * changes are needed.
 *
 * Usage: assign this as the Generator on your UGerstnerWaterWaves asset.
 * Default layers are physically motivated for open-ocean conditions.
 */
UCLASS(EditInlineNew, BlueprintType, MinimalAPI, NotBlueprintable)
class UGerstnerWaveGeneratorLayered : public UGerstnerWaterWaveGeneratorBase
{
	GENERATED_BODY()

public:
	UGerstnerWaveGeneratorLayered();

	virtual void GenerateGerstnerWaves_Implementation(TArray<FGerstnerWave>& OutWaves) const override;

	/** Seed shared across all layers. Each layer further offsets this by SeedOffset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layers")
	int32 MasterSeed = 42;

	/**
	 * Wave bands, evaluated top-to-bottom and concatenated.
	 * Default setup: [0] Ocean Swell, [1] Secondary Swell, [2] Wind Chop, [3] Capillary Ripple.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layers")
	TArray<FWaveLayerConfig> Layers;

private:
	void GenerateLayer(const FWaveLayerConfig& Layer, int32 CombinedSeed, TArray<FGerstnerWave>& OutWaves) const;
};
