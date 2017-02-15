//***************
// VMD���[�V����
//***************

#include	<stdio.h>
#include	<stdlib.h>
#include	<malloc.h>
#include	<string.h>
#include	"VMDMotion.h"


//------------------------------
// �{�[���L�[�t���[���\�[�g�p��r�֐�
//------------------------------
static int boneCompareFunc( const void *pA, const void *pB )
{
	return (int)(((BoneKeyFrame *)pA)->fFrameNo - ((BoneKeyFrame *)pB)->fFrameNo);
}

//------------------------------
// �\��L�[�t���[���\�[�g�p��r�֐�
//------------------------------
static int faceCompareFunc( const void *pA, const void *pB )
{
	return (int)(((FaceKeyFrame *)pA)->fFrameNo - ((FaceKeyFrame *)pB)->fFrameNo);
}

//================
// �R���X�g���N�^
//================
cVMDMotion::cVMDMotion( void ) : m_pMotionDataList( NULL ), m_pFaceDataList( NULL )
{
}

//==============
// �f�X�g���N�^
//==============
cVMDMotion::~cVMDMotion( void )
{
	release();
}

//====================
// �t�@�C���̓ǂݍ���
//====================
bool cVMDMotion::load( const char *szFilePath )
{
	FILE	*pFile;
	fpos_t	fposFileSize;
	unsigned char
			*pData;

	pFile = fopen( szFilePath, "rb" );
	if( !pFile )	return false;	// �t�@�C�����J���Ȃ�

	// �t�@�C���T�C�Y�擾
	fseek( pFile, 0, SEEK_END );
	fgetpos( pFile, &fposFileSize );

	// �������m��
	pData = (unsigned char *)malloc( (size_t)fposFileSize );

	// �ǂݍ���
	fseek( pFile, 0, SEEK_SET );
	fread( pData, 1, (size_t)fposFileSize, pFile );

	fclose( pFile );

	// ���[�V�����f�[�^������
	bool	bRet = initialize( pData );

	free( pData );

	return bRet;
}

//==========================
// ���[�V�����f�[�^�̏�����
//==========================
bool cVMDMotion::initialize( unsigned char *pData )
{
	release();

	// �w�b�_�̃`�F�b�N
	VMD_Header	*pVMDHeader = (VMD_Header *)pData;
	if( strncmp( pVMDHeader->szHeader, "Vocaloid Motion Data 0002", 30 ) != 0 )
		return false;	// �t�@�C���`�����Ⴄ

	pData += sizeof( VMD_Header );

	//-----------------------------------------------------
	// �{�[���̃L�[�t���[�������擾
	unsigned long	ulNumBoneKeyFrames = *((unsigned long *)pData);
	pData += sizeof( unsigned long );

	// �܂��̓��[�V�����f�[�^���̃{�[�����Ƃ̃L�[�t���[�������J�E���g
	VMD_Motion		*pVMDMotion = (VMD_Motion *)pData;
	MotionDataList	*pMotTemp;

	m_fMaxFrame = 0.0f;
	for( unsigned long i = 0 ; i < ulNumBoneKeyFrames ; i++, pVMDMotion++ )
	{
		if( m_fMaxFrame < (float)pVMDMotion->ulFrameNo )	m_fMaxFrame = (float)pVMDMotion->ulFrameNo;	// �ő�t���[���X�V

		pMotTemp = m_pMotionDataList;

		while( pMotTemp )
		{
			if( strncmp( pMotTemp->szBoneName, pVMDMotion->szBoneName, 15 ) == 0 )
			{
				// ���X�g�ɒǉ��ς݂̃{�[��
				pMotTemp->ulNumKeyFrames++;
				break;
			}
			pMotTemp = pMotTemp->pNext;
		}

		if( !pMotTemp )
		{
			// ���X�g�ɂȂ��ꍇ�͐V�K�m�[�h��ǉ�
			MotionDataList	*pNew = new MotionDataList;

			strncpy( pNew->szBoneName, pVMDMotion->szBoneName, 15 );	pNew->szBoneName[15] = '\0';
			pNew->ulNumKeyFrames = 1;

			pNew->pNext = m_pMotionDataList;
			m_pMotionDataList = pNew;
		}
	}

	// �L�[�t���[���z����m��
	pMotTemp = m_pMotionDataList;
	m_ulNumMotionNodes = 0;
	while( pMotTemp )
	{
		pMotTemp->pKeyFrames = new BoneKeyFrame[pMotTemp->ulNumKeyFrames];
		pMotTemp->ulNumKeyFrames = 0;		// �z��C���f�b�N�X�p�ɂ�������0�ɂ���
		pMotTemp = pMotTemp->pNext;

		m_ulNumMotionNodes++;
	}
	
	// �{�[�����ƂɃL�[�t���[�����i�[
	pVMDMotion = (VMD_Motion *)pData;

	for( unsigned long i = 0 ; i < ulNumBoneKeyFrames ; i++, pVMDMotion++ )
	{
		pMotTemp = m_pMotionDataList;

		while( pMotTemp )
		{
			if( strncmp( pMotTemp->szBoneName, pVMDMotion->szBoneName, 15 ) == 0 )
			{
				BoneKeyFrame	*pKeyFrame = &(pMotTemp->pKeyFrames[pMotTemp->ulNumKeyFrames]);

				pKeyFrame->fFrameNo     = (float)pVMDMotion->ulFrameNo;
				pKeyFrame->vec3Position = pVMDMotion->vec3Position;
				QuaternionNormalize( &pKeyFrame->vec4Rotation, &pVMDMotion->vec4Rotation );

				pKeyFrame->clPosXInterBezier.initialize( pVMDMotion->cInterpolationX[0], pVMDMotion->cInterpolationX[4], pVMDMotion->cInterpolationX[8], pVMDMotion->cInterpolationX[12] );
				pKeyFrame->clPosYInterBezier.initialize( pVMDMotion->cInterpolationY[0], pVMDMotion->cInterpolationY[4], pVMDMotion->cInterpolationY[8], pVMDMotion->cInterpolationY[12] );
				pKeyFrame->clPosZInterBezier.initialize( pVMDMotion->cInterpolationZ[0], pVMDMotion->cInterpolationZ[4], pVMDMotion->cInterpolationZ[8], pVMDMotion->cInterpolationZ[12] );
				pKeyFrame->clRotInterBezier.initialize( pVMDMotion->cInterpolationRot[0], pVMDMotion->cInterpolationRot[4], pVMDMotion->cInterpolationRot[8], pVMDMotion->cInterpolationRot[12] );

				pMotTemp->ulNumKeyFrames++;

				break;
			}
			pMotTemp = pMotTemp->pNext;
		}
	}

	// �L�[�t���[���z��������Ƀ\�[�g
	pMotTemp = m_pMotionDataList;

	while( pMotTemp )
	{
		qsort( pMotTemp->pKeyFrames, pMotTemp->ulNumKeyFrames, sizeof( BoneKeyFrame ), boneCompareFunc );
		pMotTemp = pMotTemp->pNext;
	}

	pData += sizeof( VMD_Motion ) * ulNumBoneKeyFrames;

	//-----------------------------------------------------
	// �\��̃L�[�t���[�������擾
	unsigned long	ulNumFaceKeyFrames = *((unsigned long *)pData);
	pData += sizeof( unsigned long );

	// ���[�V�����f�[�^���̕\��Ƃ̃L�[�t���[�������J�E���g
	VMD_Face		*pVMDFace = (VMD_Face *)pData;
	FaceDataList	*pFaceTemp;

	for( unsigned long i = 0 ; i < ulNumFaceKeyFrames ; i++, pVMDFace++ )
	{
		if( m_fMaxFrame < (float)pVMDFace->ulFrameNo )	m_fMaxFrame = (float)pVMDFace->ulFrameNo;	// �ő�t���[���X�V

		pFaceTemp = m_pFaceDataList;

		while( pFaceTemp )
		{
			if( strncmp( pFaceTemp->szFaceName, pVMDFace->szFaceName, 15 ) == 0 )
			{
				// ���X�g�ɒǉ��ς�
				pFaceTemp->ulNumKeyFrames++;
				break;
			}
			pFaceTemp = pFaceTemp->pNext;
		}

		if( !pFaceTemp )
		{
			// ���X�g�ɂȂ��ꍇ�͐V�K�m�[�h��ǉ�
			FaceDataList	*pNew = new FaceDataList;

			strncpy( pNew->szFaceName, pVMDFace->szFaceName, 15 );	pNew->szFaceName[15] = '\0';
			pNew->ulNumKeyFrames = 1;

			pNew->pNext = m_pFaceDataList;
			m_pFaceDataList = pNew;
		}
	}

	// �L�[�t���[���z����m��
	pFaceTemp = m_pFaceDataList;
	m_ulNumFaceNodes = 0;
	while( pFaceTemp )
	{
		pFaceTemp->pKeyFrames = new FaceKeyFrame[pFaceTemp->ulNumKeyFrames];
		pFaceTemp->ulNumKeyFrames = 0;		// �z��C���f�b�N�X�p�ɂ�������0�ɂ���
		pFaceTemp = pFaceTemp->pNext;

		m_ulNumFaceNodes++;
	}
	
	// �\��ƂɃL�[�t���[�����i�[
	pVMDFace = (VMD_Face *)pData;

	for( unsigned long i = 0 ; i < ulNumFaceKeyFrames ; i++, pVMDFace++ )
	{
		pFaceTemp = m_pFaceDataList;

		while( pFaceTemp )
		{
			if( strncmp( pFaceTemp->szFaceName, pVMDFace->szFaceName, 15 ) == 0 )
			{
				FaceKeyFrame	*pKeyFrame = &(pFaceTemp->pKeyFrames[pFaceTemp->ulNumKeyFrames]);

				pKeyFrame->fFrameNo = (float)pVMDFace->ulFrameNo;
				pKeyFrame->fRate    =        pVMDFace->fFactor;

				pFaceTemp->ulNumKeyFrames++;

				break;
			}
			pFaceTemp = pFaceTemp->pNext;
		}
	}

	// �L�[�t���[���z��������Ƀ\�[�g
	pFaceTemp = m_pFaceDataList;

	while( pFaceTemp )
	{
		qsort( pFaceTemp->pKeyFrames, pFaceTemp->ulNumKeyFrames, sizeof( FaceKeyFrame ), faceCompareFunc );
		pFaceTemp = pFaceTemp->pNext;
	}

	return true;
}

//======
// ���
//======
void cVMDMotion::release( void )
{
	// ���[�V�����f�[�^�̉��
	MotionDataList	*pMotTemp = m_pMotionDataList,
					*pNextMotTemp;

	while( pMotTemp )
	{
		pNextMotTemp = pMotTemp->pNext;

		delete [] pMotTemp->pKeyFrames;

		delete pMotTemp;

		pMotTemp = pNextMotTemp;
	}

	m_pMotionDataList = NULL;

	// �\��f�[�^�̉��
	FaceDataList	*pFaceTemp = m_pFaceDataList,
					*pNextFaceTemp;

	while( pFaceTemp )
	{
		pNextFaceTemp = pFaceTemp->pNext;

		delete [] pFaceTemp->pKeyFrames;

		delete pFaceTemp;

		pFaceTemp = pNextFaceTemp;
	}

	m_pFaceDataList = NULL;
}
