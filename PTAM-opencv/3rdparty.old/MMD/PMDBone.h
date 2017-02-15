//***********
// PMD�{�[��
//***********

#ifndef	_PMDBONE_H_
#define	_PMDBONE_H_

#include	"PMDTypes.h"


class cPMDBone
{
	private :
		char			m_szName[21];

		bool			m_bIKLimitAngle;	// IK���Ɋp�x���������邩�ǂ���

		Vector3			m_vec3OrgPosition;
		Vector3			m_vec3Offset;

		MMD_Matrix			m_matInvTransform;	// �����l�̃{�[�������_�Ɉړ�������悤�ȍs��

		const cPMDBone	*m_pParentBone;
		cPMDBone		*m_pChildBone;

		// �ȉ��͌��݂̒l
		Vector3			m_vec3Position;
		Vector4			m_vec4Rotation;

		MMD_Matrix			m_matLocal;

		Vector4			m_vec4LookRotation;

	public :
		MMD_Matrix			m_matSkinning;		// ���_�f�t�H�[���p�s��

		cPMDBone( void );
		~cPMDBone( void );

		void initialize( const PMD_Bone *pPMDBoneData, const cPMDBone pBoneArray[] );
		void recalcOffset( void );

		void reset( void );

		void updateMatrix( void );
		void lookAt( const Vector3 *pvecTargetPos, float fLimitXD, float fLimitXU, float fLimitY  );
		void updateSkinningMat( void );

		void debugDraw( void );

		inline const char *getName( void ){ return m_szName; }
		inline void getOrgPos( Vector3 *pvec3Out ){ *pvec3Out = m_vec3OrgPosition; }
		inline void getPos( Vector3 *pvec3Out ){ pvec3Out->x = m_matLocal[3][0]; pvec3Out->y = m_matLocal[3][1]; pvec3Out->z = m_matLocal[3][2]; }
		inline MMD_Matrix &getLocalMatrix( void ){ return m_matLocal; };

	friend class cMotionPlayer;
	friend class cPMDIK;
	friend class cPMDRigidBody;
};

#endif	// _PMDMODEL_H_
