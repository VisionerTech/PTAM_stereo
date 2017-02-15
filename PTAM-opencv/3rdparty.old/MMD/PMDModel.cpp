//***********
// PMD���f��
//***********

#include	<stdio.h>
#include	<stdlib.h>
#include	<malloc.h>
#include	<string.h>
#include	<gl/glut.h>
#include	"PMDModel.h"
#include	"TextureList.h"

#define	_DBG_BONE_DRAW		(0)
#define	_DBG_IK_DRAW		(0)
#define	_DBG_TEXTURE_DRAW	(0)

extern cTextureList		g_clsTextureList;


//--------------------------
// IK�f�[�^�\�[�g�p��r�֐�
//--------------------------
static int compareFunc( const void *pA, const void *pB )
{
	return (int)(((cPMDIK *)pA)->getSortVal() - ((cPMDIK *)pB)->getSortVal());
}

//================
// �R���X�g���N�^
//================
cPMDModel::cPMDModel( void ) : m_pvec3OrgPositionArray( NULL ), m_pvec3OrgNormalArray( NULL ), m_puvOrgTexureUVArray( NULL ),
									m_pOrgSkinInfoArray( NULL ), m_pvec3PositionArray( NULL ), m_pvec3NormalArray( NULL ), m_pIndices( NULL ),
										m_pMaterials( NULL ), m_pBoneArray( NULL ), m_pNeckBone( NULL), m_pIKArray( NULL ), m_pFaceArray( NULL )
{
}

//==============
// �f�X�g���N�^
//==============
cPMDModel::~cPMDModel( void )
{
	release();
}

//====================
// �t�@�C���̓ǂݍ���
//====================
bool cPMDModel::load( const char *szFilePath )
{
	FILE	*pFile;
	long lFileSize;
	unsigned char *pData;

	pFile = fopen( szFilePath, "rb" );
	if( !pFile )	return false;	// �t�@�C�����J���Ȃ�

	// �t�@�C���T�C�Y�擾
	fseek( pFile, 0, SEEK_END );
	lFileSize = ftell(pFile);

	// �������m��
	pData = (unsigned char *)malloc( (size_t)lFileSize );

	// �ǂݍ���
	fseek( pFile, 0, SEEK_SET );
	fread( pData, 1, (size_t)lFileSize, pFile );

	fclose( pFile );

	// ���f���f�[�^������
	bool	bRet = initialize( szFilePath, pData, lFileSize );

	free( pData );

	return bRet;
}

//======================
// ���f���f�[�^�̏�����
//======================
bool cPMDModel::initialize( const char *szFilePath, unsigned char *pData, unsigned long ulDataSize )
{
	release();
	m_clMotionPlayer.clear();

	m_bLookAt = false;

	unsigned char *pDataTop = pData;

	// �w�b�_�̃`�F�b�N
	PMD_Header	*pPMDHeader = (PMD_Header *)pData;
	if( pPMDHeader->szMagic[0] != 'P' || pPMDHeader->szMagic[1] != 'm' || pPMDHeader->szMagic[2] != 'd' || pPMDHeader->fVersion != 1.0f )
		return false;	// �t�@�C���`�����Ⴄ

	// get file path
	char filePath[256];
	int filePathLength = strrchr(szFilePath, '/')-szFilePath+1;
	strncpy(filePath, szFilePath, filePathLength);
	filePath[filePathLength] = 0x0;

	// ���f�����̃R�s�[
	strncpy( m_szModelName, pPMDHeader->szName, 20 );
	m_szModelName[20] = '\0';

	pData += sizeof( PMD_Header );

	unsigned long	ulSize;

	//-----------------------------------------------------
	// ���_���擾
	m_ulNumVertices = *((unsigned int *)pData);
	pData += sizeof( unsigned int );

	// ���_�z����R�s�[
	m_pvec3OrgPositionArray =  (Vector3 *)malloc( sizeof(Vector3)  * m_ulNumVertices );
	m_pvec3OrgNormalArray   =  (Vector3 *)malloc( sizeof(Vector3)  * m_ulNumVertices );
	m_puvOrgTexureUVArray   =    (TexUV *)malloc( sizeof(TexUV)    * m_ulNumVertices );
	m_pOrgSkinInfoArray     = (SkinInfo *)malloc( sizeof(SkinInfo) * m_ulNumVertices ); 

	PMD_Vertex	*pVertex = (PMD_Vertex *)pData;
	for( unsigned long i = 0 ; i < m_ulNumVertices ; i++, pVertex++ )
	{
		m_pvec3OrgPositionArray[i] =  pVertex->vec3Pos;
		m_pvec3OrgNormalArray[i]   =  pVertex->vec3Normal;
		m_puvOrgTexureUVArray[i]   =  pVertex->uvTex;

		m_pOrgSkinInfoArray[i].fWeight     = pVertex->cbWeight / 100.0f; 
		m_pOrgSkinInfoArray[i].unBoneNo[0] = pVertex->unBoneNo[0]; 
		m_pOrgSkinInfoArray[i].unBoneNo[1] = pVertex->unBoneNo[1]; 
	}

	ulSize = sizeof( PMD_Vertex ) * m_ulNumVertices;
	pData += ulSize;

	// �X�L�j���O�p���_�z��쐬
	m_pvec3PositionArray = (Vector3 *)malloc( sizeof(Vector3) * m_ulNumVertices );
	m_pvec3NormalArray   = (Vector3 *)malloc( sizeof(Vector3) * m_ulNumVertices );

	//-----------------------------------------------------
	// ���_�C���f�b�N�X���擾
	m_ulNumIndices = *((unsigned int *)pData);
	pData += sizeof( unsigned int );

	// ���_�C���f�b�N�X�z����R�s�[
	ulSize = sizeof( unsigned short ) * m_ulNumIndices;
	m_pIndices = (unsigned short *)malloc( ulSize );

	memcpy( m_pIndices, pData, ulSize );
	pData += ulSize;

	//-----------------------------------------------------
	// �}�e���A�����擾
	m_ulNumMaterials = *((unsigned int *)pData);
	pData += sizeof( unsigned int );

	// �}�e���A���z����R�s�[
	m_pMaterials = (Material *)malloc( sizeof(Material) * m_ulNumMaterials );

	PMD_Material	*pPMDMaterial = (PMD_Material *)pData;
	for( unsigned long i = 0 ; i < m_ulNumMaterials ; i++ )
	{
		m_pMaterials[i].col4Diffuse = pPMDMaterial->col4Diffuse;

		m_pMaterials[i].col4Specular.r = pPMDMaterial->col3Specular.r;
		m_pMaterials[i].col4Specular.g = pPMDMaterial->col3Specular.g;
		m_pMaterials[i].col4Specular.b = pPMDMaterial->col3Specular.b;
		m_pMaterials[i].col4Specular.a = 1.0f;

		m_pMaterials[i].col4Ambient.r = pPMDMaterial->col3Ambient.r;
		m_pMaterials[i].col4Ambient.g = pPMDMaterial->col3Ambient.g;
		m_pMaterials[i].col4Ambient.b = pPMDMaterial->col3Ambient.b;
		m_pMaterials[i].col4Ambient.a = 1.0f;

		m_pMaterials[i].fShininess = pPMDMaterial->fShininess;
		m_pMaterials[i].ulNumIndices = pPMDMaterial->ulNumIndices;

		if( pPMDMaterial->szTextureFileName[0] )
		{
			filePath[filePathLength] = 0x0;
			strcat(&filePath[filePathLength], pPMDMaterial->szTextureFileName);
			unsigned int dbg_num = g_clsTextureList.getTexture( filePath );
			m_pMaterials[i].uiTexID = dbg_num;
			m_pMaterials[i].bSphereMap = isSphereMapTexName( pPMDMaterial->szTextureFileName );
		}
		else
		{
			m_pMaterials[i].uiTexID = 0xFFFFFFFF;
			m_pMaterials[i].bSphereMap = false;
		}

		pPMDMaterial++;
	}

	pData += sizeof(PMD_Material) * m_ulNumMaterials;

	//-----------------------------------------------------
	// �{�[�����擾
	m_unNumBones = *((unsigned short *)pData);
	pData += sizeof( unsigned short );

	// �{�[���z����쐬
	m_pBoneArray = new cPMDBone[m_unNumBones];

	for( unsigned short i = 0 ; i < m_unNumBones ; i++ )
	{
		m_pBoneArray[i].initialize( (const PMD_Bone *)pData, m_pBoneArray );

		// ��̃{�[����ۑ�
		if( strncmp( m_pBoneArray[i].getName(), "��", 20 ) == 0 )	m_pNeckBone = &m_pBoneArray[i];

		pData += sizeof( PMD_Bone );	
	}
	for( unsigned short i = 0 ; i < m_unNumBones ; i++ )	m_pBoneArray[i].recalcOffset();

	//-----------------------------------------------------
	// IK���擾
	m_unNumIK = *((unsigned short *)pData);
	pData += sizeof( unsigned short );

	// IK�z����쐬
	if( m_unNumIK )
	{
		m_pIKArray = new cPMDIK[m_unNumIK];

		for( unsigned short i = 0 ; i < m_unNumIK ; i++ )
		{
			m_pIKArray[i].initialize( (const PMD_IK *)pData, m_pBoneArray );
			pData += sizeof( PMD_IK ) + sizeof(unsigned short) * (((PMD_IK *)pData)->cbNumLink - 1);
		}
		qsort( m_pIKArray, m_unNumIK, sizeof(cPMDIK), compareFunc );
	}

	//-----------------------------------------------------
	// �\��擾
	m_unNumFaces = *((unsigned short *)pData);
	pData += sizeof( unsigned short );

	// �\��z����쐬
	if( m_unNumFaces )
	{
		m_pFaceArray = new cPMDFace[m_unNumFaces];

		for( unsigned short i = 0 ; i < m_unNumFaces ; i++ )
		{
			m_pFaceArray[i].initialize( (const PMD_Face *)pData, &m_pFaceArray[0] );
			pData += sizeof( PMD_Face ) + sizeof(PMD_FaceVtx) * (((PMD_Face *)pData)->ulNumVertices - 1);
		}
	}

	//-----------------------------------------------------
	// �������獄�̏��܂œǂݔ�΂�

	// �\��g�p�\�����X�g
	unsigned char	cbFaceDispNum = *((unsigned char *)pData);
	pData += sizeof( unsigned char );

	pData += sizeof( unsigned short ) * cbFaceDispNum;

	// �{�[���g�p�g�����X�g
	unsigned char	cbBoneDispNum = *((unsigned char *)pData);
	pData += sizeof( unsigned char );

	pData += sizeof( char ) * 50 * cbBoneDispNum;

	// �{�[���g�p�\�����X�g
	unsigned int	ulBoneFrameDispNum = *((unsigned int *)pData);
	pData += sizeof( unsigned int );

	pData += 3 * ulBoneFrameDispNum;

	// �㑱�f�[�^�̗L�����`�F�b�N
	if( (unsigned long)pData - (unsigned long)pDataTop >= ulDataSize )	return true;

	// �p�ꖼ�Ή�
	unsigned char	cbEnglishNameExist = *((unsigned char *)pData);
	pData += sizeof( unsigned char );

	if( cbEnglishNameExist )
	{
		// ���f�����ƃR�����g(�p��)
		pData += sizeof( char ) * 276;

		// �{�[����(�p��)
		pData += sizeof( char ) * 20 * m_unNumBones;

		// �\�(�p��)
		pData += sizeof( char ) * 20 * (m_unNumFaces - 1);	// "base"�͉p�ꖼ���X�g�ɂ͊܂܂�Ȃ�

		// �{�[���g�p�g�����X�g(�p��)
		pData += sizeof( char ) * 50 * (cbBoneDispNum);
	}

	// �㑱�f�[�^�̗L�����`�F�b�N(�����̃`�F�b�N�͕s�v����)
	if( (unsigned long)pData - (unsigned long)pDataTop >= ulDataSize )	return true;

	// �g�D�[���e�N�X�`�����X�g
	pData += 100 * 10;		// �t�@�C���� 100Byte * 10��

	// �����܂œǂݔ�΂�
	//-----------------------------------------------------

	// �㑱�f�[�^�̗L�����`�F�b�N
	if( (unsigned long)pData - (unsigned long)pDataTop >= ulDataSize )	return true;

	

	

	return true;
}

//----------------------------------------------
// �X�t�B�A�}�b�v�p�̃e�N�X�`�������ǂ�����Ԃ�
//----------------------------------------------
bool cPMDModel::isSphereMapTexName( const char *szTextureName )
{
	int		iLen = strlen( szTextureName );
	bool	bRet = false;

	if( 	(szTextureName[iLen - 3] == 's' || szTextureName[iLen - 3] == 'S') &&
			(szTextureName[iLen - 2] == 'p' || szTextureName[iLen - 2] == 'P') &&
			(szTextureName[iLen - 1] == 'h' || szTextureName[iLen - 1] == 'H')		)
	{
		bRet = true;
	}

	return bRet;
}

//======================
// �w�薼�̃{�[����Ԃ�
//======================
cPMDBone *cPMDModel::getBoneByName( const char *szBoneName )
{
	cPMDBone *pBoneArray = m_pBoneArray;
	for( unsigned long i = 0 ; i < m_unNumBones ; i++, pBoneArray++ )
	{
		if( strncmp( pBoneArray->getName(), szBoneName, 20 ) == 0 )
			return pBoneArray;
	}

	return NULL;
}

//====================
// �w�薼�̕\���Ԃ�
//====================
cPMDFace *cPMDModel::getFaceByName( const char *szFaceName )
{
	cPMDFace *pFaceArray = m_pFaceArray;
	for( unsigned long i = 0 ; i < m_unNumFaces ; i++, pFaceArray++ )
	{
		if( strncmp( pFaceArray->getName(), szFaceName, 20 ) == 0 )
			return pFaceArray;
	}

	return NULL;
}

//====================
// ���[�V�����̃Z�b�g
//====================
void cPMDModel::setMotion( cVMDMotion *pVMDMotion, bool bLoop, float fInterpolateFrame )
{
	m_clMotionPlayer.setup( this, pVMDMotion, bLoop, fInterpolateFrame );
}

//==============================
// ���[�V�����ɂ��{�[���̍X�V
//==============================
bool cPMDModel::updateMotion( float fElapsedFrame )
{
	bool	bMotionEnd = false;

	// ���[�V�����X�V�O�ɕ\������Z�b�g
	if( m_pFaceArray )	m_pFaceArray[0].setFace( m_pvec3OrgPositionArray );

	// ���[�V�����X�V
	bMotionEnd = m_clMotionPlayer.update( fElapsedFrame );

	// �{�[���s��̍X�V
	for( unsigned short i = 0 ; i < m_unNumBones ; i++ )
	{
		m_pBoneArray[i].updateMatrix();
	}

	return bMotionEnd;
}

//================================
// ��̃{�[�����^�[�Q�b�g�Ɍ�����
//================================
void cPMDModel::updateNeckBone( const Vector3 *pvec3LookTarget, float fLimitXD, float fLimitXU, float fLimitY )
{
	if( m_pNeckBone && m_bLookAt )
	{
		m_pNeckBone->lookAt( pvec3LookTarget, fLimitXD, fLimitXU, fLimitY );

		unsigned short i;
		for( i = 0 ; i < m_unNumBones ; i++ )
		{
			if( m_pNeckBone == &m_pBoneArray[i] )	break;
		}

		for( ; i < m_unNumBones ; i++ )
		{
			m_pBoneArray[i].updateMatrix();
		}
	}
}

//====================
// ���_�X�L�j���O����
//====================
void cPMDModel::updateSkinning( void )
{
	// IK�̍X�V
	for( unsigned short i = 0 ; i < m_unNumIK ; i++ )
	{
		m_pIKArray[i].update();
	}

	// �X�L�j���O�p�s��̍X�V
	for( unsigned short i = 0 ; i < m_unNumBones ; i++ )
	{
		m_pBoneArray[i].updateSkinningMat();
	}

	// ���_�X�L�j���O
	MMD_Matrix	matTemp;
	for( unsigned long i = 0 ; i < m_ulNumVertices ; i++ )
	{
		if( m_pOrgSkinInfoArray[i].fWeight == 0.0f )
		{
			Vector3Transform( &m_pvec3PositionArray[i], &m_pvec3OrgPositionArray[i], m_pBoneArray[m_pOrgSkinInfoArray[i].unBoneNo[1]].m_matSkinning );
			Vector3Rotate( &m_pvec3NormalArray[i], &m_pvec3OrgNormalArray[i], m_pBoneArray[m_pOrgSkinInfoArray[i].unBoneNo[1]].m_matSkinning );
		}
		else if( m_pOrgSkinInfoArray[i].fWeight >= 0.9999f )
		{
			Vector3Transform( &m_pvec3PositionArray[i], &m_pvec3OrgPositionArray[i], m_pBoneArray[m_pOrgSkinInfoArray[i].unBoneNo[0]].m_matSkinning );
			Vector3Rotate( &m_pvec3NormalArray[i], &m_pvec3OrgNormalArray[i], m_pBoneArray[m_pOrgSkinInfoArray[i].unBoneNo[0]].m_matSkinning );
		}
		else
		{
			MatrixLerp( matTemp,	m_pBoneArray[m_pOrgSkinInfoArray[i].unBoneNo[0]].m_matSkinning,
									m_pBoneArray[m_pOrgSkinInfoArray[i].unBoneNo[1]].m_matSkinning,		m_pOrgSkinInfoArray[i].fWeight );

			Vector3Transform( &m_pvec3PositionArray[i], &m_pvec3OrgPositionArray[i], matTemp );
			Vector3Rotate( &m_pvec3NormalArray[i], &m_pvec3OrgNormalArray[i], matTemp );
		}
	}
}

//====================
// ���f���f�[�^�̕`��
//====================
void cPMDModel::render( void )
{
	if( !m_pvec3OrgPositionArray )	return;

	unsigned short	*pIndices = m_pIndices;

	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_NORMAL_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );

	// ���_���W�A�@���A�e�N�X�`�����W�̊e�z����Z�b�g
	glVertexPointer( 3, GL_FLOAT, 0, m_pvec3PositionArray );
	glNormalPointer( GL_FLOAT, 0, m_pvec3NormalArray );
	glTexCoordPointer( 2, GL_FLOAT, 0, m_puvOrgTexureUVArray );

	glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );
	glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );
	glDisable( GL_CULL_FACE );

	for( unsigned long i = 0 ; i < m_ulNumMaterials ; i++ )
	{
		// �}�e���A���ݒ�
		glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE,  (float *)&(m_pMaterials[i].col4Diffuse)  );
		glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT,  (float *)&(m_pMaterials[i].col4Ambient)  );
		glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, (float *)&(m_pMaterials[i].col4Specular) );
		glMaterialf(  GL_FRONT_AND_BACK, GL_SHININESS, m_pMaterials[i].fShininess );

		//if( m_pMaterials[i].col4Diffuse.a < 1.0f )	glDisable( GL_CULL_FACE );
		//else											glEnable( GL_CULL_FACE );

		if( m_pMaterials[i].uiTexID != 0xFFFFFFFF )
		{
			// �e�N�X�`������Ȃ�Bind����
			glEnable( GL_TEXTURE_2D );
			unsigned int dbg_num = m_pMaterials[i].uiTexID;
			glBindTexture( GL_TEXTURE_2D, dbg_num );

			if( m_pMaterials[i].bSphereMap )
			{
				glEnable( GL_TEXTURE_GEN_S );
				glEnable( GL_TEXTURE_GEN_T );
			}
			else
			{
				glDisable( GL_TEXTURE_GEN_S );
				glDisable( GL_TEXTURE_GEN_T );
			}
		}
		else
		{
			// �e�N�X�`���Ȃ�
			glDisable( GL_TEXTURE_2D );
		}

		unsigned short *dbg_p = (unsigned short *)malloc( m_pMaterials[i].ulNumIndices );
		memcpy( dbg_p, pIndices, m_pMaterials[i].ulNumIndices );
		free( dbg_p );

		// ���_�C���f�b�N�X���w�肵�ă|���S���`��
		glDrawElements( GL_TRIANGLES, m_pMaterials[i].ulNumIndices, GL_UNSIGNED_SHORT, pIndices );

		pIndices += m_pMaterials[i].ulNumIndices;
	}

	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_NORMAL_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );

	glDisable( GL_TEXTURE_GEN_S );
	glDisable( GL_TEXTURE_GEN_T );

#if	_DBG_BONE_DRAW
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_LIGHTING );

	for( unsigned short i = 0 ; i < m_unNumBones ; i++ )
	{
		m_pBoneArray[i].debugDraw();
	}

	glEnable( GL_DEPTH_TEST );
	glEnable( GL_LIGHTING );
#endif

#if	_DBG_IK_DRAW
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_LIGHTING );

	for( unsigned short i = 0 ; i < m_unNumIK ; i++ )
	{
		m_pIKArray[i].debugDraw();
	}

	glEnable( GL_DEPTH_TEST );
	glEnable( GL_LIGHTING );
#endif

#if	_DBG_TEXTURE_DRAW
	g_clsTextureList.debugDraw();
#endif
}

//========================
// ���f���f�[�^�̉e�p�`��
//========================
void cPMDModel::renderForShadow( void )
{
	if( !m_pvec3OrgPositionArray )	return;

	glEnableClientState( GL_VERTEX_ARRAY );

	glVertexPointer( 3, GL_FLOAT, 0, m_pvec3PositionArray );

	glDrawElements( GL_TRIANGLES, m_ulNumIndices, GL_UNSIGNED_SHORT, m_pIndices );

	glDisableClientState( GL_VERTEX_ARRAY );
}

//====================
// ���f���f�[�^�̉��
//====================
void cPMDModel::release( void )
{
	if( m_pvec3OrgPositionArray )
	{
		free( m_pvec3OrgPositionArray );
		m_pvec3OrgPositionArray = NULL;
	}

	if( m_pvec3OrgNormalArray )
	{
		free( m_pvec3OrgNormalArray );
		m_pvec3OrgNormalArray = NULL;
	}

	if( m_puvOrgTexureUVArray )
	{
		free( m_puvOrgTexureUVArray );
		m_puvOrgTexureUVArray = NULL;
	}

	if( m_pOrgSkinInfoArray )
	{
		free( m_pOrgSkinInfoArray );
		m_pOrgSkinInfoArray = NULL;
	}

	if( m_pvec3PositionArray )
	{
		free( m_pvec3PositionArray );
		m_pvec3PositionArray = NULL;
	}

	if( m_pvec3NormalArray )
	{
		free( m_pvec3NormalArray );
		m_pvec3NormalArray = NULL;
	}

	if( m_pIndices )
	{
		free( m_pIndices );
		m_pIndices = NULL;
	}

	if( m_pMaterials )
	{
		for( unsigned long i = 0 ; i < m_ulNumMaterials ; i++ )
		{
			if( m_pMaterials[i].uiTexID != 0xFFFFFFFF )
				g_clsTextureList.releaseTexture( m_pMaterials[i].uiTexID );
		}

		free( m_pMaterials );
		m_pMaterials = NULL;
	}

	if( m_pBoneArray )
	{
		delete [] m_pBoneArray;
		m_pBoneArray = NULL;
		m_pNeckBone = NULL;
	}

	if( m_pIKArray )
	{
		delete [] m_pIKArray;
		m_pIKArray = NULL;
	}

	if( m_pFaceArray )
	{
		delete [] m_pFaceArray;
		m_pFaceArray = NULL;
	}
}
