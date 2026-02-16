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

	/** Returns a shared two-sided unlit UMaterial for the sky dome. */
	UMaterial* GetSkyDomeMaterial();

	/** Creates a MID from the sky dome material with the given color. */
	UMaterialInstanceDynamic* CreateSkyDomeMID(UObject* Outer, FLinearColor Color);
}
