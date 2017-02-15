//***************
// PMD�\��f�[�^
//***************

#ifndef		_PMDFACE_H_
#define		_PMDFACE_H_

#include	"PMDTypes.h"


class cPMDFace
{
	private :
		char			m_szName[21];		// �\� (0x00 �I�[�C�]���� 0xFD)

		unsigned long	m_ulNumVertices;	// �\��_��

		PMD_FaceVtx		*m_pFaceVertices;	// �\��_�f�[�^

	public :
		cPMDFace( void );
		~cPMDFace( void );

		void initialize( const PMD_Face *pPMDFaceData, const cPMDFace *pPMDFaceBase );

		void setFace( Vector3 *pvec3Vertices );
		void blendFace( Vector3 *pvec3Vertices, float fRate );

		void release( void );

		inline const char *getName( void ){ return m_szName; }
};


#endif
