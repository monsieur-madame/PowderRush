#include "Effects/PowderMaterialHelper.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"

#if WITH_EDITORONLY_DATA
#include "Materials/MaterialExpressionVectorParameter.h"
#endif

namespace PowderMaterialHelper
{
	static TWeakObjectPtr<UMaterial> CachedMaterial;
	static TWeakObjectPtr<UMaterial> CachedSkyMaterial;

	UMaterial* GetColorMaterial()
	{
		if (CachedMaterial.IsValid())
		{
			return CachedMaterial.Get();
		}

		UMaterial* Mat = NewObject<UMaterial>(GetTransientPackage(), TEXT("PowderColorMaterial"), RF_Transient);

#if WITH_EDITORONLY_DATA
		UMaterialExpressionVectorParameter* Param = NewObject<UMaterialExpressionVectorParameter>(Mat);
		Param->ParameterName = TEXT("Color");
		Param->DefaultValue = FLinearColor::White;

		Mat->GetEditorOnlyData()->ExpressionCollection.AddExpression(Param);
		Mat->GetEditorOnlyData()->BaseColor.Connect(0, Param);
		Mat->PostEditChange();
#endif

		CachedMaterial = Mat;
		return Mat;
	}

	UMaterialInstanceDynamic* CreateColorMID(UObject* Outer, FLinearColor Color)
	{
		UMaterial* BaseMat = GetColorMaterial();
		if (!BaseMat)
		{
			return nullptr;
		}

		UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMat, Outer);
		if (MID)
		{
			MID->SetVectorParameterValue(TEXT("Color"), Color);
		}
		return MID;
	}

	UMaterial* GetSkyDomeMaterial()
	{
		if (CachedSkyMaterial.IsValid())
		{
			return CachedSkyMaterial.Get();
		}

		UMaterial* Mat = NewObject<UMaterial>(GetTransientPackage(), TEXT("PowderSkyDomeMaterial"), RF_Transient);
		Mat->TwoSided = true;
		Mat->SetShadingModel(MSM_Unlit);

#if WITH_EDITORONLY_DATA
		UMaterialExpressionVectorParameter* Param = NewObject<UMaterialExpressionVectorParameter>(Mat);
		Param->ParameterName = TEXT("Color");
		Param->DefaultValue = FLinearColor(0.4f, 0.65f, 0.95f);

		Mat->GetEditorOnlyData()->ExpressionCollection.AddExpression(Param);
		Mat->GetEditorOnlyData()->EmissiveColor.Connect(0, Param);
		Mat->PostEditChange();
#endif

		CachedSkyMaterial = Mat;
		return Mat;
	}

	UMaterialInstanceDynamic* CreateSkyDomeMID(UObject* Outer, FLinearColor Color)
	{
		UMaterial* BaseMat = GetSkyDomeMaterial();
		if (!BaseMat)
		{
			return nullptr;
		}

		UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMat, Outer);
		if (MID)
		{
			MID->SetVectorParameterValue(TEXT("Color"), Color);
		}
		return MID;
	}
}
