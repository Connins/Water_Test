#include "GerstnerWaveGeneratorLayered.h"

// Units: cm (UE standard).  1m = 100cm,  10m = 1000cm,  100m = 10000cm

namespace WavePresets
{
	// Primary ocean swell — long-period energy from distant storms.
	// Narrow directional spread, low steepness, few dominant waves.
	static FWaveLayerConfig MakeSwell()
	{
		FWaveLayerConfig L;
		L.NumWaves           = 1;
		L.SeedOffset         = 0;
		L.WindAngleDeg       = 0.f;
		L.DirectionSpreadDeg = 8.f;
		L.MinWavelength      = 8000.f;   // 80 m
		L.MaxWavelength      = 25000.f;  // 250 m
		L.WavelengthFalloff  = 2.f;
		L.MinAmplitude       = 0.f;
		L.MaxAmplitude       = 360.f;
		L.AmplitudeFalloff   = 2.f;
		L.LargeSteepness     = 0.f;
		L.SmallSteepness     = 0.f;
		L.SteepnessFalloff   = 1.f;
		L.Randomness         = 0.3f;
		L.AmplitudeScale     = 1.f;
		return L;
	}

	// Secondary swell — shorter period, slightly different direction.
	static FWaveLayerConfig MakeSecondarySwell()
	{
		FWaveLayerConfig L;
		L.NumWaves           = 6;
		L.SeedOffset         = 100;
		L.WindAngleDeg       = 25.f;
		L.DirectionSpreadDeg = 25.f;
		L.MinWavelength      = 2500.f;   // 25 m
		L.MaxWavelength      = 8000.f;   // 80 m
		L.WavelengthFalloff  = 2.f;
		L.MinAmplitude       = 1.f;
		L.MaxAmplitude       = 5.f;
		L.AmplitudeFalloff   = 2.f;
		L.LargeSteepness     = 0.f;
		L.SmallSteepness     = 0.f;
		L.SteepnessFalloff   = 1.f;
		L.Randomness         = 0.5f;
		L.AmplitudeScale     = 1.f;
		return L;
	}
}

// -----------------------------------------------------------------------------

UGerstnerWaveGeneratorLayered::UGerstnerWaveGeneratorLayered()
{
	Layers.Add(WavePresets::MakeSwell());
	Layers.Add(WavePresets::MakeSecondarySwell());
}

void UGerstnerWaveGeneratorLayered::GenerateGerstnerWaves_Implementation(TArray<FGerstnerWave>& OutWaves) const
{
	ensure(OutWaves.Num() == 0);

	for (const FWaveLayerConfig& Layer : Layers)
	{
		if (Layer.bEnabled && Layer.NumWaves > 0)
		{
			GenerateLayer(Layer, MasterSeed + Layer.SeedOffset, OutWaves);
		}
	}
}

void UGerstnerWaveGeneratorLayered::GenerateLayer(const FWaveLayerConfig& Layer, int32 CombinedSeed, TArray<FGerstnerWave>& OutWaves) const
{
	FRandomStream Rng(CombinedSeed);
	const float InvN = 1.f / FMath::Max(Layer.NumWaves, 1);

	for (int32 i = 0; i < Layer.NumWaves; ++i)
	{
		const float Jitter = Rng.FRandRange(-Layer.Randomness * InvN, Layer.Randomness * InvN);
		const float Alpha  = FMath::Clamp(1.f - (float)i * InvN + Jitter, 0.f, 1.f);

		FGerstnerWave& Wave = OutWaves.AddDefaulted_GetRef();

		FMath::SinCos(&Wave.Direction.Y, &Wave.Direction.X,
			FMath::DegreesToRadians(Layer.WindAngleDeg));

		if (i > 0)
		{
			Wave.Direction = Wave.Direction.RotateAngleAxis(
				Rng.FRandRange(-Layer.DirectionSpreadDeg, Layer.DirectionSpreadDeg),
				FVector::UpVector);
		}

		Wave.WaveLength = FMath::Lerp(
			Layer.MinWavelength,
			Layer.MaxWavelength,
			FMath::Pow(Alpha, Layer.WavelengthFalloff));

		Wave.Amplitude = FMath::Max(
			FMath::Lerp(Layer.MinAmplitude, Layer.MaxAmplitude,
				FMath::Pow(Alpha, Layer.AmplitudeFalloff)) * Layer.AmplitudeScale,
			0.0001f);

		Wave.Steepness = FMath::Lerp(
			Layer.LargeSteepness,
			Layer.SmallSteepness,
			FMath::Pow((float)i * InvN, Layer.SteepnessFalloff));

		// Recompute() is called by UGerstnerWaterWaves::RecomputeWaves() after
		// this generator returns — do not call it here.
	}
}
