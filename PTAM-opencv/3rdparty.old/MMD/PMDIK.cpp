//*******
// PMDIK
//*******

#include	<gl/glut.h>
#include	<stdio.h>
#include	<string.h>
#include	<math.h>
#include	"PMDIK.h"


//================
// �R���X�g���N�^
//================
cPMDIK::cPMDIK( void ) : m_ppBoneList( NULL )
{
}

//==============
// �f�X�g���N�^
//==============
cPMDIK::~cPMDIK( void )
{
	release();
}

//========
// ������
//========
void cPMDIK::initialize( const PMD_IK *pPMDIKData, cPMDBone pBoneArray[] )
{
	release();


	// IK�^�[�Q�b�g�{�[��
	m_pTargetBone = &(pBoneArray[pPMDIKData->nTargetNo]);

	// IK��[�{�[��
	m_pEffBone = &(pBoneArray[pPMDIKData->nEffNo]);

	m_unCount = pPMDIKData->unCount;
	m_fFact = pPMDIKData->fFact * 3.1415926f;
	m_nSortVal = pPMDIKData->punLinkNo[0];

	// IK�����N�z��̍쐬
	m_cbNumLink = pPMDIKData->cbNumLink;
	m_ppBoneList = new cPMDBone *[m_cbNumLink];
	for( unsigned char i = 0 ; i < m_cbNumLink ; i++ )
	{
		m_ppBoneList[i] = &(pBoneArray[ pPMDIKData->punLinkNo[i] ]);	// �{�[���ԍ��͍~���Ŋi�[����Ă���
		if( strncmp( m_ppBoneList[i]->getName(), "���Ђ�", 20 ) == 0 ||  strncmp( m_ppBoneList[i]->getName(), "�E�Ђ�", 20 ) == 0 )
		{
			m_ppBoneList[i]->m_bIKLimitAngle = true;
		}
	}
}

//======
// �X�V
//======
void cPMDIK::update( void )
{
	Vector3	vec3OrgTargetPos;
	vec3OrgTargetPos.x = m_pTargetBone->m_matLocal[3][0];
	vec3OrgTargetPos.y = m_pTargetBone->m_matLocal[3][1];
	vec3OrgTargetPos.z = m_pTargetBone->m_matLocal[3][2];

	Vector3	vec3EffPos;
	Vector3	vec3TargetPos;

	for( short i = m_cbNumLink - 1 ; i >= 0 ; i-- ){ m_ppBoneList[i]->updateMatrix(); }
	m_pEffBone->updateMatrix();

	for( unsigned short it = 0 ; it < m_unCount ; it++ )
	{
		for( unsigned char cbLinkIdx = 0 ; cbLinkIdx < m_cbNumLink ; cbLinkIdx++ )
		{
			// �G�t�F�N�^�̈ʒu�̎擾
			vec3EffPos.x = m_pEffBone->m_matLocal[3][0];
			vec3EffPos.y = m_pEffBone->m_matLocal[3][1];
			vec3EffPos.z = m_pEffBone->m_matLocal[3][2];

			// ���[���h���W�n���璍�ڃm�[�h�̋Ǐ�(���[�J��)���W�n�ւ̕ϊ�
			MMD_Matrix	matInvBone;
			MatrixInverse( matInvBone, m_ppBoneList[cbLinkIdx]->m_matLocal );

			// �G�t�F�N�^�C���B�ڕW�̃��[�J���ʒu
			Vector3Transform( &vec3EffPos, &vec3EffPos, matInvBone );
			Vector3Transform( &vec3TargetPos, &vec3OrgTargetPos, matInvBone );

			// �\���߂���ΏI��
			Vector3	vec3Diff;
			Vector3Sub( &vec3Diff, &vec3EffPos, &vec3TargetPos );
			if( Vector3DotProduct( &vec3Diff, &vec3Diff ) < 0.0000001f )	return;

			// (1) ��֐߁��G�t�F�N�^�ʒu�ւ̕����x�N�g��
			Vector3Normalize( &vec3EffPos, &vec3EffPos );

			// (2) ��֐߁��ڕW�ʒu�ւ̕����x�N�g��
			Vector3Normalize( &vec3TargetPos, &vec3TargetPos );

			// �x�N�g�� (1) �� (2) �Ɉ�v�����邽�߂̍ŒZ��]�ʁiAxis-Angle�j
			//
			// ��]�p
			float	fRotAngle = acosf( Vector3DotProduct( &vec3EffPos, &vec3TargetPos ) );

			if( 0.00000001f < fabsf( fRotAngle ) )
			{
				if( fRotAngle < -m_fFact )	fRotAngle = -m_fFact;
				else if( m_fFact < fRotAngle )	fRotAngle = m_fFact;

				// ��]��
				Vector3 vec3RotAxis;

				Vector3CrossProduct( &vec3RotAxis, &vec3EffPos, &vec3TargetPos );
				if( Vector3DotProduct( &vec3RotAxis, &vec3RotAxis ) < 0.0000001f )	continue;

				Vector3Normalize( &vec3RotAxis, &vec3RotAxis );

				// �֐߉�]�ʂ̕␳
				Vector4		vec4RotQuat;
				QuaternionCreateAxis( &vec4RotQuat, &vec3RotAxis, fRotAngle );

				if( m_ppBoneList[cbLinkIdx]->m_bIKLimitAngle )	limitAngle( &vec4RotQuat, &vec4RotQuat );

				QuaternionNormalize( &vec4RotQuat, &vec4RotQuat );

				QuaternionMultiply( &m_ppBoneList[cbLinkIdx]->m_vec4Rotation, &m_ppBoneList[cbLinkIdx]->m_vec4Rotation, &vec4RotQuat );
				QuaternionNormalize( &m_ppBoneList[cbLinkIdx]->m_vec4Rotation, &m_ppBoneList[cbLinkIdx]->m_vec4Rotation );

				for( short i = cbLinkIdx ; i >= 0 ; i-- ){ m_ppBoneList[i]->updateMatrix(); }
				m_pEffBone->updateMatrix();
			}
		}
	}
}

//----------------------------
// �{�[���̉�]�p�x�𐧌�����
//----------------------------
void cPMDIK::limitAngle( Vector4 *pvec4Out, const Vector4 *pvec4Src )
{
	Vector3	vec3Angle;

	// XYZ����]�̎擾
	QuaternionToEuler( &vec3Angle, pvec4Src );

	// �p�x����
	if( vec3Angle.x < -3.14159f )	vec3Angle.x = -3.14159f;
	if( -0.002f < vec3Angle.x )		vec3Angle.x = -0.002f;
	vec3Angle.y = 0.0f;
	vec3Angle.z = 0.0f;

	// XYZ����]����N�H�[�^�j�I����
	QuaternionCreateEuler( pvec4Out, &vec3Angle );
}

//==============
// �f�o�b�O�`��
//==============
void cPMDIK::debugDraw( void )
{
	for( unsigned char i = 0 ; i < m_cbNumLink ; i++ )
	{
		glPushMatrix();

			glMultMatrixf( (const float *)m_ppBoneList[i]->m_matLocal );

			glColor4f( 0.0f, 1.0f, 1.0f, 1.0f );

			glDisable( GL_TEXTURE_2D );
			glutSolidCube( 0.3f );

		glPopMatrix();

		glPushMatrix();

			glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

			if( m_ppBoneList[i]->m_pParentBone )
			{
				glBegin( GL_LINES );
					glVertex3f( m_ppBoneList[i]->m_pParentBone->m_matLocal[3][0], m_ppBoneList[i]->m_pParentBone->m_matLocal[3][1], m_ppBoneList[i]->m_pParentBone->m_matLocal[3][2] );
					glVertex3f(                m_ppBoneList[i]->m_matLocal[3][0],                m_ppBoneList[i]->m_matLocal[3][1],                m_ppBoneList[i]->m_matLocal[3][2] );
				glEnd();
			}

		glPopMatrix();
	}

	glPushMatrix();

		glMultMatrixf( (const float *)m_pEffBone->m_matLocal );

		glColor4f( 1.0f, 1.0f, 0.0f, 1.0f );

		glDisable( GL_TEXTURE_2D );
		glutSolidCube( 0.3f );

	glPopMatrix();

	glPushMatrix();

		glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

		if( m_pEffBone->m_pParentBone )
		{
			glBegin( GL_LINES );
				glVertex3f( m_pEffBone->m_pParentBone->m_matLocal[3][0], m_pEffBone->m_pParentBone->m_matLocal[3][1], m_pEffBone->m_pParentBone->m_matLocal[3][2] );
				glVertex3f(                m_pEffBone->m_matLocal[3][0],                m_pEffBone->m_matLocal[3][1],                m_pEffBone->m_matLocal[3][2] );
			glEnd();
		}

	glPopMatrix();

	glPushMatrix();

		glMultMatrixf( (const float *)m_pTargetBone->m_matLocal );

		glColor4f( 1.0f, 0.0f, 0.0f, 1.0f );

		glDisable( GL_TEXTURE_2D );
		glutSolidCube( 0.3f );

	glPopMatrix();
}

//======
// ���
//======
void cPMDIK::release( void )
{
	if( m_ppBoneList )
	{
		delete [] m_ppBoneList;
		m_ppBoneList = NULL;
	}
}
