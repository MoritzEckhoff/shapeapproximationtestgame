// Copyright Epic Games, Inc. All Rights Reserved.

#include "Sampling/MeshPropertyMapEvaluator.h"
#include "Sampling/MeshMapBaker.h"
#include "Util/ColorConstants.h"

using namespace UE::Geometry;

void FMeshPropertyMapEvaluator::Setup(const FMeshBaseBaker& Baker, FEvaluationContext& Context)
{
	// Cache data from the baker
	DetailSampler = Baker.GetDetailSampler();
	auto GetDetailNormalMaps = [this](const void* Mesh)
	{
		const FDetailNormalTexture* NormalMap = DetailSampler->GetNormalMap(Mesh);
		if (NormalMap)
		{
			// Require valid normal map, UV layer and tangents to enable normal map transfer.
			const bool bEnableNormalMapTransfer = DetailSampler->HasUVs(Mesh, NormalMap->Get<1>()) && DetailSampler->HasTangents(Mesh);
			if (bEnableNormalMapTransfer)
			{
				DetailNormalTextures.Add(Mesh, *NormalMap);
				bHasDetailNormalTextures = true;
			}
		}
	};
	DetailSampler->ProcessMeshes(GetDetailNormalMaps);

	Context.Evaluate = bHasDetailNormalTextures ? &EvaluateSample<true> : &EvaluateSample<false>;
	Context.EvaluateDefault = &EvaluateDefault;
	Context.EvaluateColor = &EvaluateColor;
	Context.EvalData = this;
	Context.AccumulateMode = EAccumulateMode::Add;
	Context.DataLayout = { EComponents::Float3 };

	Bounds = DetailSampler->GetBounds();
	for (int32 j = 0; j < 3; ++j)
	{
		if (Bounds.Diagonal()[j] < FMathf::ZeroTolerance)
		{
			Bounds.Min[j] = Bounds.Center()[j] - FMathf::ZeroTolerance;
			Bounds.Max[j] = Bounds.Center()[j] + FMathf::ZeroTolerance;
		}
	}

	DefaultValue = FVector3f::Zero();
	switch (this->Property)
	{
	case EMeshPropertyMapType::Position:
		DefaultValue = PositionToColor(Bounds.Center(), Bounds);
		break;
	case EMeshPropertyMapType::FacetNormal:
		DefaultValue = NormalToColor(FVector3d::UnitZ());
		break;
	case EMeshPropertyMapType::Normal:
		DefaultValue = NormalToColor(FVector3d::UnitZ());
		break;
	case EMeshPropertyMapType::UVPosition:
		DefaultValue = UVToColor(FVector2f::Zero());
		break;
	case EMeshPropertyMapType::MaterialID:
		DefaultValue = FVector3f(LinearColors::LightPink3f());
		break;
	case EMeshPropertyMapType::VertexColor:
		DefaultValue = FVector3f::One();
		break;
	}
}

template <bool bUseDetailNormalMap>
void FMeshPropertyMapEvaluator::EvaluateSample(float*& Out, const FCorrespondenceSample& Sample, void* EvalData)
{
	const FMeshPropertyMapEvaluator* Eval = static_cast<FMeshPropertyMapEvaluator*>(EvalData);
	const FVector3f SampleResult = Eval->SampleFunction<bUseDetailNormalMap>(Sample);
	WriteToBuffer(Out, SampleResult);
}

void FMeshPropertyMapEvaluator::EvaluateDefault(float*& Out, void* EvalData)
{
	WriteToBuffer(Out, FVector3f::Zero());
}

void FMeshPropertyMapEvaluator::EvaluateColor(const int DataIdx, float*& In, FVector4f& Out, void* EvalData)
{
	// TODO: Move property color space transformation from EvaluateSample/Default to here.
	Out = FVector4f(In[0], In[1], In[2], 1.0f);
	In += 3;
}

template <bool bUseDetailNormalMap>
FVector3f FMeshPropertyMapEvaluator::SampleFunction(const FCorrespondenceSample& SampleData) const
{
	const void* DetailMesh = SampleData.DetailMesh;
	const FVector3d& DetailBaryCoords = SampleData.DetailBaryCoords;
	FVector3f Color = DefaultValue;
	const int32 DetailTriID = SampleData.DetailTriID;

	switch (this->Property)
	{
	case EMeshPropertyMapType::Position:
	{
		const FVector3d Position = DetailSampler->TriBaryInterpolatePoint(DetailMesh, DetailTriID, DetailBaryCoords);
		Color = PositionToColor(Position, Bounds);
	}
	break;
	case EMeshPropertyMapType::FacetNormal:
	{
		const FVector3d FacetNormal = DetailSampler->GetTriNormal(DetailMesh, DetailTriID);
		Color = NormalToColor(FacetNormal);
	}
	break;
	case EMeshPropertyMapType::Normal:
	{
		FVector3f DetailNormal;
		if (DetailSampler->TriBaryInterpolateNormal(DetailMesh, DetailTriID, DetailBaryCoords, DetailNormal))
		{
			Normalize(DetailNormal);

			if constexpr (bUseDetailNormalMap)
			{
				const TImageBuilder<FVector4f>* DetailNormalMap = nullptr;
				int DetailNormalUVLayer = 0;
				const FDetailNormalTexture* DetailNormalTexture = DetailNormalTextures.Find(DetailMesh);
				if (DetailNormalTexture)
				{
					Tie(DetailNormalMap, DetailNormalUVLayer) = *DetailNormalTexture;
				}

				if (DetailNormalMap)
				{
					FVector3d DetailTangentX, DetailTangentY;
					DetailSampler->TriBaryInterpolateTangents(
						DetailMesh,
						SampleData.DetailTriID,
						SampleData.DetailBaryCoords,
						DetailTangentX, DetailTangentY);
	
					FVector2f DetailUV;
					DetailSampler->TriBaryInterpolateUV(DetailMesh, DetailTriID, DetailBaryCoords, DetailNormalUVLayer, DetailUV);
					const FVector4f DetailNormalColor4 = DetailNormalMap->BilinearSampleUV<float>(FVector2d(DetailUV), FVector4f(0, 0, 0, 1));

					// Map color space [0,1] to normal space [-1,1]
					const FVector3f DetailNormalColor(DetailNormalColor4.X, DetailNormalColor4.Y, DetailNormalColor4.Z);
					const FVector3f DetailNormalTangentSpace = (DetailNormalColor * 2.0f) - FVector3f::One();

					// Convert detail normal tangent space to object space
					FVector3f DetailNormalObjectSpace = DetailNormalTangentSpace.X * FVector3f(DetailTangentX) + DetailNormalTangentSpace.Y * FVector3f(DetailTangentY) + DetailNormalTangentSpace.Z * DetailNormal;
					Normalize(DetailNormalObjectSpace);
					DetailNormal = DetailNormalObjectSpace;
				}
			}

			Color = NormalToColor(FVector3d(DetailNormal));
		}
	}
	break;
	case EMeshPropertyMapType::UVPosition:
	{
		FVector2f DetailUV;
		if (DetailSampler->TriBaryInterpolateUV(DetailMesh, DetailTriID, DetailBaryCoords, 0, DetailUV))
		{
			Color = UVToColor(DetailUV);
		}
	}
	break;
	case EMeshPropertyMapType::MaterialID:
	{
		const int32 MatID = DetailSampler->GetMaterialID(DetailMesh, DetailTriID);
		Color = LinearColors::SelectColor<FVector3f>(MatID);
	}
	break;
	case EMeshPropertyMapType::VertexColor:
	{
		FVector4f DetailColor;
		if (DetailSampler->TriBaryInterpolateColor(DetailMesh, DetailTriID, DetailBaryCoords, DetailColor))
		{
			Color = FVector3f(DetailColor.X, DetailColor.Y, DetailColor.Z);
		}
	}
	break;
	}
	return Color;
}


