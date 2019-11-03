
#include "../valve_sdk/csgostructs.hpp"
#include "../features/MaterialManager.hpp"

enum class ChamsModes : int
{
	NORMAL,
	FLAT,
	WIREFRAME,
	GLASS,
	METALLIC,
	XQZ,
	METALLIC_XQZ,
	FLAT_XQZ
};

#pragma once
class Chams : public Singleton<Chams>
{
public:
	matrix3x4_t BoneMatrix[128];
	void OnSceneEnd();
	void zzVectorRotate(const float* in1, const matrix3x4_t& in2, float* out);
	void zVectorRotate(const Vector& in1, const matrix3x4_t& in2, Vector& out);
	void VectorRotate(const Vector& in1, const QAngle& in2, Vector& out);
	void MatrixCopy(const matrix3x4_t& source, matrix3x4_t& target);
	void MatrixMultiply(matrix3x4_t& in1, const matrix3x4_t& in2);
	void zAngleMatrix(const Vector angles, matrix3x4_t& matrix);
	void OnDrawModelExecute(void* pResults, DrawModelInfo_t* pInfo, matrix3x4_t* pBoneToWorld, float* flpFlexWeights,
		float* flpFlexDelayedWeights, Vector& vrModelOrigin, int iFlags);
};

