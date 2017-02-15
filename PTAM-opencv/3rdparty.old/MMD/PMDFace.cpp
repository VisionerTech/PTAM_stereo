//***************
// PMD�\��f�[�^
//***************

#include	<gl/glut.h>
#include	<stdio.h>
#include	<malloc.h>
#include	<string.h>
#include	"PMDFace.h"

//================
// �R���X�g���N�^
//================
cPMDFace::cPMDFace( void ) : m_pFaceVertices( NULL )
{
}

//==============
// �f�X�g���N�^
//==============
cPMDFace::~cPMDFace( void )
{
	release();
}

//========
// ������
//========
void cPMDFace::initialize( const PMD_Face *pPMDFaceData, const cPMDFace *pPMDFaceBase )
{
	release();


	// �\��̃R�s�[
	strncpy( m_szName, pPMDFaceData->szName, 20 );
	m_szName[20] = '\0';

	// �\��_�f�[�^�̃R�s�[
	m_ulNumVertices = pPMDFaceData->ulNumVertices;

	m_pFaceVertices = (PMD_FaceVtx *)malloc( sizeof(PMD_FaceVtx) * m_ulNumVertices );
	memcpy( m_pFaceVertices, pPMDFaceData->pVertices, sizeof(PMD_FaceVtx) * m_ulNumVertices );

	// base�Ƃ̑��΃C���f�b�N�X���΃C���f�b�N�X�ɕϊ�
	if( pPMDFaceBase && pPMDFaceBase != this )
	{
		PMD_FaceVtx	*pVertex = m_pFaceVertices;
		for( unsigned long i = 0 ; i < m_ulNumVertices ; i++, pVertex++ )
		{
			Vector3Add( &pVertex->vec3Pos, &pPMDFaceBase->m_pFaceVertices[pVertex->ulIndex].vec3Pos, &pVertex->vec3Pos );
			pVertex->ulIndex = pPMDFaceBase->m_pFaceVertices[pVertex->ulIndex].ulIndex;
		}
	}
}

//============
// �\���K�p
//============
void cPMDFace::setFace( Vector3 *pvec3Vertices )
{
	if( !m_pFaceVertices )	return;

	PMD_FaceVtx		*pVertex = m_pFaceVertices;
	for( unsigned long i = 0 ; i < m_ulNumVertices ; i++, pVertex++ )
	{
		pvec3Vertices[pVertex->ulIndex] = pVertex->vec3Pos;
	}
}

//======================
// �\����u�����h�œK�p
//======================
void cPMDFace::blendFace( Vector3 *pvec3Vertices, float fRate )
{
	if( !m_pFaceVertices )	return;

	PMD_FaceVtx		*pVertex = m_pFaceVertices;
	unsigned long	ulIndex;
	for( unsigned long i = 0 ; i < m_ulNumVertices ; i++, pVertex++ )
	{
		ulIndex = pVertex->ulIndex;
		Vector3Lerp( &pvec3Vertices[ulIndex], &pvec3Vertices[ulIndex], &pVertex->vec3Pos, fRate );
	}
}

//======
// ���
//======
void cPMDFace::release( void )
{
	if( m_pFaceVertices )
	{
		free( m_pFaceVertices );
		m_pFaceVertices = NULL;
	}
}
