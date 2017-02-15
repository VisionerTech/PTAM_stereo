//************************************
// �x�N�g���A�s��A�N�H�[�^�j�I�����Z
//************************************

#ifndef		_VECMATQUAT_H_
#define		_VECMATQUAT_H_

#pragma pack( push, 1 )

// 3�v�f�x�N�g��
struct Vector3
{
	float	x, y, z;
};

// 4�v�f�x�N�g��(�N�H�[�^�j�I�����p)
struct Vector4
{
	float	x, y, z, w;
};

// �e�N�X�`��UV
struct TexUV
{
	float	u, v;
};

// �A���t�@�����J���[
struct Color3
{
	float	r, g, b;
};

// �A���t�@����J���[
struct Color4
{
	float	r, g, b, a;
};

// �s��
typedef float	MMD_Matrix[4][4];

#pragma pack( pop )


extern void Vector3Add( Vector3 *pvec3Out, const Vector3 *pvec3Add1, const Vector3 *pvec3Add2 );
extern void Vector3Sub( Vector3 *pvec3Dst, const Vector3 *pvec3Sub1, const Vector3 *pvec3Sub2 );
extern void Vector3MulAdd( Vector3 *pvec3Out, const Vector3 *pvec3Add1, const Vector3 *pvec3Add2, float fRate );
extern void Vector3Normalize( Vector3 *pvec3Out, const Vector3 *pvec3Src );
extern float Vector3DotProduct( const Vector3 *pvec3Src1, const Vector3 *pvec3Src2 );
extern void Vector3CrossProduct( Vector3 *pvec3Out, const Vector3 *pvec3Src1, const Vector3 *pvec3Src2 );
extern void Vector3Lerp( Vector3 *pvec3Out, const Vector3 *pvec3Src1, const Vector3 *pvec3Src2, float fLerpValue );
extern void Vector3Transform( Vector3 *pvec3Out, const Vector3 *pVec3In, const MMD_Matrix matTransform );
extern void Vector3Rotate( Vector3 *pvec3Out, const Vector3 *pVec3In, const MMD_Matrix matRotate );

extern void MatrixIdentity( MMD_Matrix matOut );
extern void MatrixRotationX( MMD_Matrix matOut, float fAngle );
extern void MatrixMultiply( MMD_Matrix matOut, const MMD_Matrix matSrc1, const MMD_Matrix matSrc2 );
extern void MatrixInverse( MMD_Matrix matOut, const MMD_Matrix matSrc );
extern void MatrixLerp( MMD_Matrix matOut, MMD_Matrix matSrc1, MMD_Matrix matSrc2, float fLerpValue );
extern void MatrixPlanarProjection( MMD_Matrix matOut, const Vector4 *pvec4Plane, const Vector3 *pvec3LightPos );

extern void QuaternionCreateAxis( Vector4 *pvec4Out, const Vector3 *pvec3Axis, float fRotAngle );
extern void QuaternionCreateEuler( Vector4 *pvec4Out, const Vector3 *pvec3EulerAngle );
extern void QuaternionNormalize( Vector4 *pvec4Out, const Vector4 *pvec4Src );
extern void QuaternionMultiply( Vector4 *pvec4Out, const Vector4 *pvec4Src1, const Vector4 *pvec4Src2 );
extern void QuaternionSlerp( Vector4 *pvec4Out, const Vector4 *pvec4Src1, const Vector4 *pvec4Src2, float fLerpValue );
extern void QuaternionToMatrix( MMD_Matrix matOut, const Vector4 *pvec4Quat );
extern void QuaternionToEuler( Vector3 *pvecAngle, const Vector4 *pvec4Quat );

#endif	// _VECMATQUAT_H_
