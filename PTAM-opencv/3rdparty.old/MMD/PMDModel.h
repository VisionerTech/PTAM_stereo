//***********
// PMD���f��
//***********

#ifndef	_PMDMODEL_H_
#define	_PMDMODEL_H_

#include	"PMDTypes.h"
#include	"PMDBone.h"
#include	"PMDIK.h"
#include	"PMDFace.h"
#include	"MotionPlayer.h"

class cPMDModel
{
	private :
		char			m_szModelName[21];	// ���f����

		unsigned long	m_ulNumVertices;	// ���_��

		Vector3			*m_pvec3OrgPositionArray;	// ���W�z��
		Vector3			*m_pvec3OrgNormalArray;		// �@���z��
		TexUV			*m_puvOrgTexureUVArray;		// �e�N�X�`�����W�z��

		struct SkinInfo
		{
			float			fWeight;		// �E�F�C�g
			unsigned short	unBoneNo[2];	// �{�[���ԍ�
		};
		SkinInfo		*m_pOrgSkinInfoArray;

		Vector3			*m_pvec3PositionArray;
		Vector3			*m_pvec3NormalArray;

		unsigned long	m_ulNumIndices;		// ���_�C���f�b�N�X��
		unsigned short	*m_pIndices;		// ���_�C���f�b�N�X�z��

		struct Material
		{
			Color4			col4Diffuse,
							col4Specular,
							col4Ambient;
			float			fShininess;

			unsigned long	ulNumIndices;
			unsigned int	uiTexID;		// ?????ID
			bool			bSphereMap;		// ??????????????
		};
		unsigned long	m_ulNumMaterials;	// �}�e���A����
		Material		*m_pMaterials;		// �}�e���A���z��

		unsigned short	m_unNumBones;		// �{�[����
		cPMDBone		*m_pBoneArray;		// �{�[���z��

		cPMDBone		*m_pNeckBone;		// ��̃{�[��
		bool			m_bLookAt;			// ����^�[�Q�b�g�֌����邩�ǂ���

		unsigned short	m_unNumIK;			// IK��
		cPMDIK			*m_pIKArray;		// IK�z��

		unsigned short	m_unNumFaces;		// �\�
		cPMDFace		*m_pFaceArray;		// �\��z��

		cMotionPlayer	m_clMotionPlayer;


		bool isSphereMapTexName( const char *szTextureName );

	public :
		cPMDModel( void );
		~cPMDModel( void );

		bool load( const char *szFilePath );
		bool initialize( const char *szFilePath, unsigned char *pData, unsigned long ulDataSize );

		cPMDBone *getBoneByName( const char *szBoneName );
		cPMDFace *getFaceByName( const char *szFaceName );

		void setMotion( cVMDMotion *pVMDMotion, bool bLoop = false, float fInterpolateFrame = 0.0f );

		bool updateMotion( float fElapsedFrame );
		void updateNeckBone( const Vector3 *pvec3LookTarget, float fLimitXD = -20.0f, float fLimitXU = 45.0f, float fLimitY = 80.0f );
		void updateSkinning( void );

		void render( void );
		void renderForShadow( void );

		void release( void );

		inline void toggleLookAtFlag( void ){ m_bLookAt = !m_bLookAt; } 
		inline void setLookAtFlag( bool bFlag ){ m_bLookAt = bFlag; } 

		inline const char *getModelName( void ){ return m_szModelName; } 

	friend class cMotionPlayer;
};

#endif	// _PMDMODEL_H_
