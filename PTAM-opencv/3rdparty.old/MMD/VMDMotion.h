//***************
// VMD���[�V����
//***************

#ifndef		_VMDMOTION_H_
#define		_VMDMOTION_H_

#include	"PMDTypes.h"
#include	"PMDBone.h"
#include	"VMDTypes.h"
#include	"VMDBezier.h"

// �{�[���Ώۂ̃L�[�t���[���f�[�^
struct BoneKeyFrame
{
	float	fFrameNo;		// �t���[���ԍ�

	Vector3	vec3Position;	// �ʒu
	Vector4	vec4Rotation;	// ��](�N�H�[�^�j�I��)

	cVMDBezier	clPosXInterBezier;	// X���ړ����
	cVMDBezier	clPosYInterBezier;	// Y���ړ����
	cVMDBezier	clPosZInterBezier;	// Z���ړ����
	cVMDBezier	clRotInterBezier;	// ��]���
};

// �{�[�����Ƃ̃L�[�t���[���f�[�^�̃��X�g
struct MotionDataList
{
	char	szBoneName[16];			// �{�[����

	unsigned long	ulNumKeyFrames;	// �L�[�t���[����
	BoneKeyFrame	*pKeyFrames;	// �L�[�t���[���f�[�^�z��

	MotionDataList	*pNext;
};

// �\��̃L�[�t���[���f�[�^
struct FaceKeyFrame
{
	float	fFrameNo;		// �t���[���ԍ�
	float	fRate;			// �t�����h��
};

// �\��Ƃ̃L�[�t���[���f�[�^�̃��X�g
struct FaceDataList
{
	char	szFaceName[16];	// �\�

	unsigned long	ulNumKeyFrames;	// �L�[�t���[����
	FaceKeyFrame	*pKeyFrames;	// �L�[�t���[���f�[�^�z��

	FaceDataList	*pNext;
};


class cVMDMotion
{
	private :
		MotionDataList		*m_pMotionDataList;	// �{�[�����Ƃ̃L�[�t���[���f�[�^�̃��X�g
		unsigned long		m_ulNumMotionNodes;	// �{�[�����[�V�����̃m�[�h��

		FaceDataList		*m_pFaceDataList;	// �\��Ƃ̃L�[�t���[���f�[�^�̃��X�g
		unsigned long		m_ulNumFaceNodes;	// �\��[�V�����̃m�[�h��

		float				m_fMaxFrame;		// �Ō�̃t���[���ԍ�

	public :
		cVMDMotion( void );
		~cVMDMotion( void );

		bool load( const char *szFilePath );
		bool initialize( unsigned char *pData );

		void release( void );

		inline MotionDataList *getMotionDataList( void ){ return m_pMotionDataList; }
		inline unsigned long getNumMotionNodes( void ){ return m_ulNumMotionNodes; }

		inline FaceDataList *getFaceDataList( void ){ return m_pFaceDataList; }
		inline unsigned long getNumFaceNodes( void ){ return m_ulNumFaceNodes; }

		inline float getMaxFrame( void ){ return m_fMaxFrame; }
};

#endif	// _VMDMOTION_H_
