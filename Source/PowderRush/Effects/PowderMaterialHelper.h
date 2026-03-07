#pragma once
#include "CoreMinimal.h"

class UMaterial;
class UMaterialInstanceDynamic;

namespace PowderMaterialHelper
{
	/** Returns a shared transient UMaterial with a "Color" vector parameter wired to BaseColor. */
	UMaterial* GetColorMaterial();

	/** Creates a MID from the shared color material with the given color applied. */
	UMaterialInstanceDynamic* CreateColorMID(UObject* Outer, FLinearColor Color);

	/** Returns a shared Lit UMaterial for snow terrain (SnowColor + Roughness parameters). */
	UMaterial* GetSnowTerrainMaterial();

	/** Creates a MID from the snow terrain material with the given color and roughness. */
	UMaterialInstanceDynamic* CreateSnowTerrainMID(UObject* Outer, FLinearColor SnowColor, float Roughness = 0.85f);

	/** Returns a shared translucent unlit UMaterial with a "Color" vector parameter and "Opacity" scalar. */
	UMaterial* GetTranslucentColorMaterial();

	/** Creates a MID from the translucent material with the given color and opacity. */
	UMaterialInstanceDynamic* CreateTranslucentColorMID(UObject* Outer, FLinearColor Color, float Opacity = 0.5f);

	/** Returns a shared two-sided unlit UMaterial for the sky dome. */
	UMaterial* GetSkyDomeMaterial();

	/** Creates a MID from the sky dome material with the given color. */
	UMaterialInstanceDynamic* CreateSkyDomeMID(UObject* Outer, FLinearColor Color);
}
